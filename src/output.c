#include "output.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_ext_workspace_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/util/log.h>

#include "layer.h"
#include "list_helpers.h"
#include "pointer.h"
#include "rules.h"
#include "toplevel.h"
#include "util/macros.h"
#include "util/memory.h"
#include "util/time_util.h"
#include "workspace.h"

static inline void
update_area(struct state *state, struct output *output) {
    int width, height;
    wlr_output_effective_resolution(output->wlr_output, &width, &height);

    output->full_area = (struct wlr_box){
            .x = output->output_layout_output->x,
            .y = output->output_layout_output->y,
            .width = width,
            .height = height,
    };

    output->usable_area = output->full_area;
    layers_arrange(state, output);
}

static inline bool
mode_fit_and_better(struct wlr_output_mode *mode, int width, int height, int refresh, struct wlr_output_mode *best) {
    if(mode->width != width && mode->height != height) {
        return false;
    }

    return !best || abs(mode->refresh - refresh) < abs(best->refresh - refresh);
}

static struct wlr_output_mode *
find_mode(struct output *output, int width, int height, int refresh) {
    struct wlr_output_mode *mode = NULL;

    struct wlr_output_mode *iter;
    wl_list_for_each(iter, &output->wlr_output->modes, link) {
        if(mode_fit_and_better(iter, width, height, refresh, mode)) {
            mode = iter;
        }
    }

    if(mode) {
        return mode;
    }

    return wlr_output_preferred_mode(output->wlr_output);
}

static struct wlr_output_mode *
find_highest_refresh(struct output *output, int width, int height) {
    struct wlr_output_mode *mode = NULL;

    struct wlr_output_mode *iter;
    wl_list_for_each(iter, &output->wlr_output->modes, link) {
        if(iter->width == width && iter->height == height && (mode == NULL || iter->refresh > mode->refresh)) {
            mode = iter;
        }
    }

    if(mode) {
        return mode;
    }

    return wlr_output_preferred_mode(output->wlr_output);
}

static void
modeset(struct output *output, int width, int height, int refresh, float scale) {
    bool wants_preferred = width == 0 || height == 0;
    bool wants_highest_refresh = refresh == 0;

    struct wlr_output_mode *mode = wants_preferred       ? wlr_output_preferred_mode(output->wlr_output)
                                 : wants_highest_refresh ? find_highest_refresh(output, width, height)
                                                         : find_mode(output, width, height, refresh);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    if(scale > 0) {
        wlr_output_state_set_scale(&state, scale);
    }
    if(mode) {
        wlr_log(WLR_INFO, "modesetting output '%s' to %dx%d@%dmHz", output->wlr_output->name, mode->width, mode->height,
                mode->refresh);
        wlr_output_state_set_mode(&state, mode);
    } else {
        wlr_log(WLR_INFO, "output '%s' has no modes available", output->wlr_output->name);
    }

    // try to commit the state. it should not fail!
    if(!wlr_output_commit_state(output->wlr_output, &state)) {
        wlr_log(WLR_ERROR, "could not modeset the output '%s'", output->wlr_output->name);
    }

    wlr_output_state_finish(&state);
}

static void
handle_frame(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct output *output = CONTAINER_OF(listener, struct output, frame);

    wlr_scene_output_commit(output->scene_output, NULL);

    struct timespec now = time_now_timespec();
    wlr_scene_output_send_frame_done(output->scene_output, &now);
}

static void
handle_request_state(struct wl_listener *listener, void *data) {
    struct output *output = CONTAINER_OF(listener, struct output, request_state);
    struct wlr_output_event_request_state *request_state = data;
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "request state event for output '%s':", output->wlr_output->name);
    wlr_output_commit_state(output->wlr_output, request_state->state);

    update_area(state, output);
    layers_arrange_all(state);
}

static void
destroy_layers(struct output *output) {
    struct layer *iter, *tmp;
    wl_list_for_each_safe(iter, tmp, &output->layers.overlay, link) {
        layer_destroy(iter);
    }

    wl_list_for_each_safe(iter, tmp, &output->layers.top, link) {
        layer_destroy(iter);
    }

    wl_list_for_each_safe(iter, tmp, &output->layers.bottom, link) {
        layer_destroy(iter);
    }

    wl_list_for_each_safe(iter, tmp, &output->layers.background, link) {
        layer_destroy(iter);
    }
}

static void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct output *output = CONTAINER_OF(listener, struct output, destroy);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "destroying output '%s'", output->wlr_output->name);

    struct workspace *iter, *tmp;
    wl_list_for_each_safe(iter, tmp, &output->workspaces, link) {
        workspace_destroy(state, iter, true);
    }

    destroy_layers(output);

    if(output->lock_rect) {
        wlr_scene_node_destroy(&output->lock_rect->node);
    }

    wlr_ext_workspace_group_handle_v1_destroy(output->ext_workspace_group);

    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);

    free(output);
}

static bool
matches_rule(const char *name, struct output_rule *rule) {
    if((rule->fields & OUTPUT_RULE_FIELD_MATCH_NAME) && rule->match.name && !strstr(name, rule->match.name)) {
        return false;
    }

    return true;
}

static void
create_config(struct state *state, const char *name, struct output_rule *config) {
    struct output_rule *iter;
    wl_list_for_each(iter, &state->config.output_rules, link) {
        if(!matches_rule(name, iter)) {
            continue;
        }

        if(iter->fields & OUTPUT_RULE_FIELD_X) {
            config->x = iter->x;
            // we only care if these are specified since not specifying them means to add it to the layout automatically
            config->fields |= OUTPUT_RULE_FIELD_X;
        }
        if(iter->fields & OUTPUT_RULE_FIELD_Y) {
            config->y = iter->y;
            config->fields |= OUTPUT_RULE_FIELD_Y;
        }
        if(iter->fields & OUTPUT_RULE_FIELD_WIDTH) {
            config->width = iter->width;
        }
        if(iter->fields & OUTPUT_RULE_FIELD_HEIGHT) {
            config->height = iter->height;
        }
        if(iter->fields & OUTPUT_RULE_FIELD_REFRESH_RATE) {
            config->refresh_rate = iter->refresh_rate;
        }
        if(iter->fields & OUTPUT_RULE_FIELD_SCALE) {
            config->scale = iter->scale;
        }
    }
}

void
output_configure_from_rules(struct state *state, struct output *output) {
    struct output_rule config = {0};
    create_config(state, output->wlr_output->name, &config);

    modeset(output, config.width, config.height, config.refresh_rate, config.scale);
    if(config.fields & OUTPUT_RULE_FIELD_X && config.fields & OUTPUT_RULE_FIELD_Y) {
        wlr_log(WLR_INFO, "placing output '%s' at %d, %d", output->wlr_output->name, config.x, config.y);
        output->output_layout_output =
                wlr_output_layout_add(state->output_layout, output->wlr_output, config.x, config.y);
    } else {
        wlr_log(WLR_INFO, "placing output '%s' automatically", output->wlr_output->name);
        output->output_layout_output = wlr_output_layout_add_auto(state->output_layout, output->wlr_output);
    }

    update_area(state, output);
}

struct output *
output_create(struct state *state, struct wlr_output *wlr_output) {
    struct output *output = ALLOC(struct output);
    output->wlr_output = wlr_output;
    wlr_output->data = output;

    wlr_log(WLR_DEBUG, "new output '%s'", wlr_output->name);

    output->ext_workspace_group = wlr_ext_workspace_group_handle_v1_create(state->ext_workspace_mgr.wlr_mgr,
            EXT_WORKSPACE_GROUP_HANDLE_V1_GROUP_CAPABILITIES_CREATE_WORKSPACE);
    wlr_ext_workspace_group_handle_v1_output_enter(output->ext_workspace_group, wlr_output);
    output->ext_workspace_group->data = output;

    // create the dummy workspace. TODO: find orphans when hotplugging
    wl_list_init(&output->workspaces);
    output->dummy_workspace = workspace_create(state, output, -1);

    // insert it into the global list
    wl_list_insert(&state->outputs, &output->link);

    // initialize per output layers on this output
    wl_list_init(&output->layers.background);
    wl_list_init(&output->layers.bottom);
    wl_list_init(&output->layers.top);
    wl_list_init(&output->layers.overlay);

    wlr_output_init_render(wlr_output, state->backend.allocator, state->backend.renderer);
    output->scene_output = wlr_scene_output_create(state->scene.wlr_scene, wlr_output);

    output_configure_from_rules(state, output);
    wlr_scene_output_layout_add_output(state->scene.scene_layout, output->output_layout_output, output->scene_output);

    // TODO: when locking
    // output->lock_rect = wlr_scene_rect_create(&state->scene.wlr_scene->tree, 0, 0, (float[4]){0.0f, 0.0f,
    // 0.0f, 1.0f}); wlr_scene_node_place_above(&output->session_lock_rect->node, &server.overlay_tree->node);
    // wlr_scene_node_set_enabled(&output->session_lock_rect->node, server.mode == SERVER_MODE_LOCKED);

    output->frame.notify = handle_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    output->request_state.notify = handle_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    return output;
}

void
output_focus(struct state *state, struct output *output) {
    // go from the top most tree and find the view that accepts keyboard focus
    {
        struct layer *iter;
        wl_list_for_each(iter, &output->layers.overlay, link) {
            if(iter->wlr_layer->current.keyboard_interactive) {
                layer_focus(state, iter);
                return;
            }
        }
        wl_list_for_each(iter, &output->layers.top, link) {
            if(iter->wlr_layer->current.keyboard_interactive) {
                layer_focus(state, iter);
                return;
            }
        }
    }

    struct workspace *workspace = output->active_workspace;
    state->active_workspace = workspace;

    if(workspace->fullscreen) {
        toplevel_focus(state, workspace->fullscreen);
        return;
    }

    struct wl_list *top_most = wl_list_first(&workspace->floats);
    if(top_most) {
        struct toplevel *toplevel = CONTAINER_OF(top_most, struct toplevel, link);
        toplevel_focus(state, toplevel);
        return;
    }

    if(workspace->master) {
        toplevel_focus(state, workspace->master);
        return;
    }

    // lookup bottom and backgroud layers. note: there cant be any slaves if there is no master
    {
        struct layer *iter;
        wl_list_for_each(iter, &output->layers.bottom, link) {
            if(iter->wlr_layer->current.keyboard_interactive) {
                layer_focus(state, iter);
                return;
            }
        }
        wl_list_for_each(iter, &output->layers.background, link) {
            if(iter->wlr_layer->current.keyboard_interactive) {
                layer_focus(state, iter);
                return;
            }
        }
    }
}

struct output *
output_find_by_name(struct state *state, char *name) {
    struct output *iter;
    wl_list_for_each(iter, &state->outputs, link) {
        if(strcmp(iter->wlr_output->name, name) == 0) {
            return iter;
        }
    }

    return NULL;
}

// bool
// output_transfer_existing_workspaces(struct mwc_output *output) {
//   /* if this output is reconnected then its workspaces are on some other monitor,
//    * we try to find it; this is not efficient as things could be flagged, i am just lazy rn */
//   bool found = false;
//   struct mwc_output *o;
//   struct mwc_workspace *w, *tmp;
//   wl_list_for_each(o, &server.outputs, link) {
//     wl_list_for_each_safe(w, tmp, &o->workspaces, link) {
//       if(w->config != NULL && strcmp(w->config->output, output->wlr_output->name) == 0) {
//         /* fix that outputs state */
//         if(w == o->active_workspace) {
//           struct mwc_workspace *owned_workspace = output_find_owned_workspace(o);
//           /* it should have had its own workspace */
//           assert(owned_workspace != NULL);
//           change_workspace(owned_workspace, false);
//         }
//         /* transfer it to this output */
//         w->output = output;
//         wl_list_remove(&w->link);
//         wl_list_insert(&output->workspaces, &w->link);
//         if(output->active_workspace == NULL) {
//           output->active_workspace = w;
//         }
//         found = true;
//       }
//     }
//   }
//
//   return found;
//
// }
//
