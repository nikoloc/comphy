#include "output.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "layer.h"
#include "list_helpers.h"
#include "pointer.h"
#include "toplevel.h"
#include "util/macros.h"
#include "util/memory.h"
#include "util/time_util.h"
#include "workspace.h"

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
modeset(struct output *output, int width, int height, int refresh, double scale) {
    bool wants_preferred = width == 0 || height == 0;
    bool wants_highest_refresh = refresh == 0;

    struct wlr_output_mode *mode = wants_preferred       ? wlr_output_preferred_mode(output->wlr_output)
                                 : wants_highest_refresh ? find_highest_refresh(output, width, height)
                                                         : find_mode(output, width, height, refresh);

    if(!mode) {
        wlr_log(WLR_INFO, "output '%s' has no modes available", output->wlr_output->name);
        return;
    }

    wlr_log(WLR_INFO, "modesetting output '%s' to %dx%d@%dmHz", output->wlr_output->name, mode->width, mode->height,
            mode->refresh);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    wlr_output_state_set_scale(&state, scale);
    wlr_output_state_set_mode(&state, mode);

    // try to commit the state. it should not fail!
    if(!wlr_output_commit_state(output->wlr_output, &state)) {
        wlr_log(WLR_ERROR, "could not modeset the output '%s'", output->wlr_output->name);
    }

    wlr_output_state_finish(&state);
}

static void
handle_frame(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct output *output = CONTAINER_OF(listener, struct output, destroy);

    wlr_scene_output_commit(output->scene_output, NULL);

    struct timespec now = time_now_timespec();
    wlr_scene_output_send_frame_done(output->scene_output, &now);
}

static void
handle_request_state(struct wl_listener *listener, void *data) {
    struct output *output = CONTAINER_OF(listener, struct output, request_state);

    struct wlr_output_event_request_state *request_state = data;
    wlr_output_commit_state(output->wlr_output, request_state->state);
}

static void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct output *output = CONTAINER_OF(listener, struct output, destroy);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "destroying output '%s'", output->wlr_output->name);

    if(!state->is_exiting) {
        struct wl_list *next = wl_list_get_next_or_prev(&state->outputs, &output->link);
        if(next) {
            struct output *next_output = CONTAINER_OF(next, struct output, link);
            // TODO: orphans
            // struct workspace *w, *tmp;
            // wl_list_for_each_safe(w, tmp, &output->workspaces, link) {
            //     w->output = new;
            //     wl_list_remove(&w->link);
            //     wl_list_insert(&new->workspaces, &w->link);
            //     layout_set_pending_state(w);
            // }

            enum view *view = view_get_focused(state);
            if(view) {
                // TODO: check if the layers and lock surfaces are destroyed before hand
                struct output *focused_output = view_get_output(view);
                if(focused_output == output) {
                    // if the focus is on this output move it elsewhere
                    output_focus(state, next_output);
                }
            }
        }
    }

    if(output->lock_rect) {
        wlr_scene_node_destroy(&output->lock_rect->node);
    }

    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);

    free(output);
}

struct output *
output_create(struct state *state, struct wlr_output *wlr_output) {
    struct output *output = ALLOC(struct output);
    output->wlr_output = wlr_output;
    wlr_output->data = output;

    wlr_log(WLR_DEBUG, "new output '%s'", output->wlr_output->name);

    wlr_output_init_render(wlr_output, state->backend.allocator, state->backend.renderer);

    // TODO: not needed anymore?
    // struct output_config config = {0};
    // output_configure(state, output, &config);

    output->scene_output = wlr_scene_output_create(state->scene.wlr_scene, wlr_output);
    output->output_layout_output = wlr_output_layout_add_auto(state->output_layout, output->wlr_output);

    output->full_area = (struct wlr_box){
            output->output_layout_output->x,
            output->output_layout_output->y,
            output->wlr_output->width,
            output->wlr_output->height,
    };
    output->usable_area = output->full_area;

    // TODO: when locking
    // output->lock_rect = wlr_scene_rect_create(&state->scene.wlr_scene->tree, 0, 0, (float[4]){0.0f, 0.0f,
    // 0.0f, 1.0f}); wlr_scene_node_place_above(&output->session_lock_rect->node, &server.overlay_tree->node);
    // wlr_scene_node_set_enabled(&output->session_lock_rect->node, server.mode == SERVER_MODE_LOCKED);

    // create the dummy workspace. TODO: find orphans when hotplugging
    wl_list_init(&output->workspaces);
    output->dummy_workspace = workspace_create(state, output, 1);
    output->active_workspace = output->dummy_workspace;
    wl_list_insert(&output->workspaces, &output->dummy_workspace->link);

    // initialize per output layers on this output
    wl_list_init(&output->layer.background);
    wl_list_init(&output->layer.bottom);
    wl_list_init(&output->layer.top);
    wl_list_init(&output->layer.overlay);

    // insert it into the global list
    wl_list_insert(&state->outputs, &output->link);

    output->frame.notify = handle_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    output->request_state.notify = handle_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    return output;
}

void
output_configure(struct state *state, struct output *output, struct output_config *config) {
    modeset(output, config->width, config->height, config->refresh, config->scale);

    // if() {
    //     return wlr_output_layout_add_auto(g.wlr_output_layout, output->wlr_output);
    // }

    // TODO: this will work for now, but add the configuration through `wlr_randr` tool
    output->output_layout_output = wlr_output_layout_add(state->output_layout, output->wlr_output, 0, 0);

    output->full_area = (struct wlr_box){
            output->output_layout_output->x,
            output->output_layout_output->y,
            output->wlr_output->width,
            output->wlr_output->height,
    };

    output->usable_area = output->full_area;
}

void
output_focus(struct state *state, struct output *output) {
    // go from the top most tree and find the view that accepts keyboard focus
    {
        struct layer *iter;
        wl_list_for_each(iter, &output->layer.overlay, link) {
            if(iter->wlr_layer->current.keyboard_interactive) {
                layer_focus(state, iter);
                return;
            }
        }
        wl_list_for_each(iter, &output->layer.top, link) {
            if(iter->wlr_layer->current.keyboard_interactive) {
                layer_focus(state, iter);
                return;
            }
        }
    }

    struct workspace *workspace = output->active_workspace;
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
        wl_list_for_each(iter, &output->layer.bottom, link) {
            if(iter->wlr_layer->current.keyboard_interactive) {
                layer_focus(state, iter);
                return;
            }
        }
        wl_list_for_each(iter, &output->layer.background, link) {
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
