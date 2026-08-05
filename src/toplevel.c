#include "toplevel.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_fractional_scale_v1.h>
#include <wlr/types/wlr_xdg_activation_v1.h>
#include <wlr/util/edges.h>
#include <wlr/util/log.h>

#include "box_helpers.h"
#include "layout.h"
#include "list_helpers.h"
#include "output.h"
#include "pointer.h"
#include "transaction.h"
#include "util/macros.h"
#include "util/memory.h"
#include "util/time_util.h"
#include "workspace.h"

static bool
should_float(struct state *state, struct toplevel *toplevel) {
    // we make toplevels float if they have fixed size or are children of another toplevel
    bool natural = (toplevel->wlr_toplevel->current.max_height > 0 &&
                           toplevel->wlr_toplevel->current.max_height == toplevel->wlr_toplevel->current.min_height) ||
                   (toplevel->wlr_toplevel->current.max_width > 0 &&
                           toplevel->wlr_toplevel->current.max_width == toplevel->wlr_toplevel->current.min_width) ||
                   toplevel->wlr_toplevel->parent;

    if(natural) {
        return true;
    }

    // TODO: go through the rules
    // struct window_rule_float *w;
    // wl_list_for_each(w, &server.config->window_rules.floating, link) {
    //     if(toplevel_matches_window_rule(toplevel, &w->condition)) {
    //         return true;
    //     }
    // }

    return false;
}

static void
center_float(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    struct wlr_box output_box = toplevel->workspace->output->usable_area;
    toplevel->pending.x = output_box.x + (output_box.width - toplevel->pending.width) / 2;
    toplevel->pending.y = output_box.y + (output_box.height - toplevel->pending.height) / 2;

    toplevel->needs_centering = false;
}

static enum toplevel_state
default_state(struct state *state, struct toplevel *toplevel) {
    if(should_float(state, toplevel)) {
        return TOPLEVEL_STATE_FLOAT;
    }

    return TOPLEVEL_STATE_TILED;
}

static void
set_tiled_hacks(struct toplevel *toplevel, bool is_tiled) {
    // in order for the clients to react better to our demands in tiled mode, we tell it that its maximized and tiled on
    // all sides. this makes the client not draw stuff such as rounded corners, shadows and usually means the client
    // will respect our desired size more than if not
    wlr_xdg_toplevel_set_maximized(toplevel->wlr_toplevel, is_tiled);

    if(is_tiled) {
        wlr_xdg_toplevel_set_tiled(toplevel->wlr_toplevel,
                WLR_EDGE_TOP | WLR_EDGE_RIGHT | WLR_EDGE_BOTTOM | WLR_EDGE_LEFT);
    } else {
        wlr_xdg_toplevel_set_tiled(toplevel->wlr_toplevel, WLR_EDGE_NONE);
    }
}

static void
handle_map(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, map);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "toplevel '%p' mapped", (void *)toplevel);

    if(!state->active_workspace) {
        // TODO: what if there is no output?
        return;
    }

    struct workspace *workspace = state->active_workspace;
    toplevel->workspace = workspace;

    // create the scene stuff
    toplevel->content_tree = wlr_scene_xdg_surface_create(toplevel->scene_tree, toplevel->wlr_toplevel->base);
    wlr_scene_node_set_position(&toplevel->content_tree->node, state->config.border.width, state->config.border.width);

    float color[4];
    color_to_wlr_color(state->config.border.color.inactive, color);
    toplevel->border = wlr_scene_rect_create(toplevel->scene_tree, 0, 0, color);
    wlr_scene_node_lower_to_bottom(&toplevel->border->node);

    // in order to obtain this toplevel we keep a pointer to view, from which the type of view can be read, and then
    // extracted by using `CONTAINER_OF()`
    toplevel->scene_tree->node.data = &toplevel->view;

    switch(toplevel->state) {
        case TOPLEVEL_STATE_TILED: {
            layout_add(workspace, toplevel);
            // immediately reconfigure the layout so the right size is sent to the client. NOTE: this is not ideal,
            // since we request another state from the client, but working around it creates a lot more work i am not
            // doing rn. current way needs disabling the node before the first transaction commit tho.
            layout_configure(state, workspace);
            wlr_scene_node_set_enabled(&toplevel->scene_tree->node, false);
            toplevel->needs_initial_enable = true;
            break;
        }
        case TOPLEVEL_STATE_FLOAT: {
            // reparent the tree to floating global
            wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.floats);

            // respect its choosen size
            struct wlr_box *geometry = &toplevel->wlr_toplevel->base->geometry;
            toplevel->pending.width = geometry->width;
            toplevel->pending.height = geometry->height;

            if(toplevel->needs_centering) {
                center_float(toplevel);
            }

            // here we dont need the extra configure, just commit the state as is
            transaction_commit(state, toplevel);
            break;
        }
        default: {
            // we do not allow for clients to be fullscreened before they are mapped
            UNREACHABLE();
        }
    }

    toplevel_focus(state, toplevel);
}

static struct toplevel *
find_next_to_focus_from_prev(struct toplevel *toplevel) {
    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        struct wl_list *next = wl_list_next_or_prev(&toplevel->workspace->floats, &toplevel->link);
        if(next) {
            return CONTAINER_OF(next, struct toplevel, link);
        }

        // might be NULL
        return toplevel->workspace->master;
    }

    ASSERT(toplevel->state == TOPLEVEL_STATE_TILED && "should not be called for fullscreen toplevels");

    if(toplevel == toplevel->workspace->master) {
        // this is master
        struct wl_list *bottom_slave = wl_list_last(&toplevel->workspace->slaves);
        if(bottom_slave) {
            return CONTAINER_OF(bottom_slave, struct toplevel, link);
        }

        // nothing left
        return NULL;
    }

    struct wl_list *next = wl_list_next_or_prev(&toplevel->workspace->slaves, &toplevel->link);
    if(next) {
        return CONTAINER_OF(next, struct toplevel, link);
    }

    return toplevel->workspace->master;
}

static void
handle_unmap(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, unmap);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "toplevel '%p' unmapped", (void *)toplevel);

    struct workspace *workspace = toplevel->workspace;

    if(toplevel == state->prev_focused) {
        // remove the invalid pointer
        state->prev_focused = NULL;
    }

    if(toplevel == state->focused_toplevel) {
        state->focused_toplevel = NULL;
    }

    // turn off its scene nodes
    wlr_scene_node_set_enabled(&toplevel->scene_tree->node, false);

    if(toplevel == state->grabbed_toplevel) {
        operation_stop_whatever(state);

        struct output *output = cursor_get_output(state);
        if(!output) {
            return;
        }

        // find the next thing to give focus to
        output_focus(state, output);
        return;
    }

    // before removing the toplevel from the layout etc, find the one to give the focus next
    struct toplevel *focus_next = find_next_to_focus_from_prev(toplevel);

    switch(toplevel->state) {
        case TOPLEVEL_STATE_TILED: {
            layout_remove(toplevel);
            layout_configure(state, toplevel->workspace);
            break;
        }
        case TOPLEVEL_STATE_FLOAT: {
            // if floating just remove him from the list
            wl_list_remove(&toplevel->link);
            break;
        }
        case TOPLEVEL_STATE_FULLSCREEN: {
            workspace->fullscreen = NULL;
            output_focus(state, workspace->output);
            return;
        }
    }

    if(focus_next) {
        toplevel_focus(state, focus_next);
    }
}

static void
send_frame_done(struct toplevel *toplevel) {
    struct timespec now = time_now_timespec();

    wlr_surface_send_frame_done(toplevel->wlr_toplevel->base->surface, &now);
}

static void
handle_commit(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, commit);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "toplevel '%p' commited", (void *)toplevel);

    if(!toplevel->wlr_toplevel->base->initialized) {
        return;
    }

    if(toplevel->wlr_toplevel->base->initial_commit) {
        toplevel->state = default_state(state, toplevel);

        // on initial commit we need to tell the client the initial size; we only do so if there are rules, elso we just
        // tell it to choose its own size
        int width = 0, height = 0;
        if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
            // TODO: actaully lookup rules
            // toplevel_floating_size(toplevel, &width, &height);
            toplevel->needs_centering = true;
        }

        wlr_xdg_toplevel_set_size(toplevel->wlr_toplevel, width, height);
        wlr_xdg_toplevel_set_wm_capabilities(toplevel->wlr_toplevel, WLR_XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN);
        set_tiled_hacks(toplevel, toplevel->state == TOPLEVEL_STATE_TILED);
        return;
    }

    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        // we conform to the size it chose, even if its not the one we requested, or we did not request it at all
        struct wlr_box *geometry = &toplevel->wlr_toplevel->base->geometry;
        toplevel->pending.width = geometry->width;
        toplevel->pending.height = geometry->height;
        wlr_log(WLR_DEBUG, "toplevel '%p' changed size to %dx%d", (void *)toplevel, geometry->width, geometry->height);

        if(toplevel->needs_centering) {
            center_float(toplevel);
        }

        transaction_commit(state, toplevel);
        return;
    }

    if(!toplevel->is_dirty) {
        wlr_log(WLR_DEBUG, "toplevel commited but not dirty");
        return;
    }

    u32 serial = toplevel->wlr_toplevel->base->current.configure_serial;
    if(serial < toplevel->configure_serial) {
        wlr_log(WLR_DEBUG, "toplevel commited but old serial");
        // send a frame event, this makes the client commit a new buffer, conforming to our new state, in order to
        // conform to our transaction state. kinda hacky, but thats just how clients operate under wayland
        send_frame_done(toplevel);
        return;
    }

    // commit the new state for this toplevel. this will check for all the other toplevels on the screen and finalize
    // the state for the output if everything is perfect, else its going to wait for others
    transaction_commit(state, toplevel);
}

static void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, destroy);

    if(toplevel->transaction_schedule_idle) {
        wl_event_source_remove(toplevel->transaction_schedule_idle);
    }

    // we need to manually destroy it since we manually created it. note: this is also going to destroy the snapshot
    // tree if it exists
    wlr_scene_node_destroy(&toplevel->scene_tree->node);
    wlr_foreign_toplevel_handle_v1_destroy(toplevel->foreign_toplevel_handle);

    wl_list_remove(&toplevel->map.link);
    wl_list_remove(&toplevel->unmap.link);
    wl_list_remove(&toplevel->commit.link);
    wl_list_remove(&toplevel->destroy.link);
    wl_list_remove(&toplevel->request_move.link);
    wl_list_remove(&toplevel->request_resize.link);
    wl_list_remove(&toplevel->request_maximize.link);
    wl_list_remove(&toplevel->request_fullscreen.link);
    wl_list_remove(&toplevel->set_app_id.link);
    wl_list_remove(&toplevel->set_title.link);

    free(toplevel);
}

static void
handle_request_move(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, request_move);
    struct state *state = state_get();

    enum view *pointer_focused = seat_get_pointer_focused(state);
    if(!pointer_focused || toplevel != view_get_toplevel(pointer_focused)) {
        return;
    }

    operation_start_move(state, toplevel);
}

static void
handle_request_resize(struct wl_listener *listener, void *data) {
    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    struct state *state = state_get();

    enum view *pointer_focused = seat_get_pointer_focused(state);
    if(!pointer_focused || toplevel != view_get_toplevel(pointer_focused)) {
        return;
    }

    operation_start_resize(state, toplevel, event->edges);
}

static void
handle_request_maximize(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, request_maximize);

    // no op
    if(toplevel->wlr_toplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(toplevel->wlr_toplevel->base);
    }
}

static void
handle_request_fullscreen(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, request_fullscreen);
    struct state *state = state_get();

    if(!toplevel->wlr_toplevel->base->surface->mapped) {
        // cant fullscreen an unmapped toplevel
        return;
    }

    if(toplevel->wlr_toplevel->requested.fullscreen) {
        toplevel_set_fullscreen(state, toplevel, true);
    } else {
        toplevel_set_fullscreen(state, toplevel, false);
    }
}

static void
handle_set_app_id(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, set_app_id);
    wlr_foreign_toplevel_handle_v1_set_app_id(toplevel->foreign_toplevel_handle, toplevel->wlr_toplevel->app_id);
}

static void
handle_set_title(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, set_title);
    wlr_foreign_toplevel_handle_v1_set_title(toplevel->foreign_toplevel_handle, toplevel->wlr_toplevel->title);
}

static void
send_scale(struct toplevel *toplevel, float scale) {
    wlr_fractional_scale_v1_notify_scale(toplevel->wlr_toplevel->base->surface, scale);
    // fallback for the clients not supporting the fractional scale protocol
    wlr_surface_set_preferred_buffer_scale(toplevel->wlr_toplevel->base->surface, ceil(scale));
}

struct toplevel *
toplevel_create(struct state *state, struct wlr_xdg_toplevel *wlr_toplevel) {
    struct toplevel *toplevel = ALLOC(struct toplevel);
    toplevel->wlr_toplevel = wlr_toplevel;
    wlr_toplevel->base->data = toplevel;

    toplevel->view = VIEW_TOPLEVEL;
    // we create the tree in the tiled tree, and swap it later if necessery. NOTE: disable the tree, see notes in
    // `handle_map()`
    toplevel->scene_tree = wlr_scene_tree_create(state->scene.trees.tiled);
    toplevel->scene_tree->node.data = &toplevel->view;

    if(state->active_workspace) {
        // if there is an output we try and guess this is going to be the output this toplevel is going to be
        // displayed on. this might change if the user changes the workspace for example, or for any other reason,
        // but is a good guess most of the time. we send this outputs scale info to the client, so it can map the
        // surface with that scale in mind, resulting in less flicker
        struct output *current_output = state->active_workspace->output;
        send_scale(toplevel, current_output->wlr_output->scale);
    }

    toplevel->foreign_toplevel_handle = wlr_foreign_toplevel_handle_v1_create(state->foreign_toplevel_manager);

    toplevel->map.notify = handle_map;
    wl_signal_add(&wlr_toplevel->base->surface->events.map, &toplevel->map);

    toplevel->unmap.notify = handle_unmap;
    wl_signal_add(&wlr_toplevel->base->surface->events.unmap, &toplevel->unmap);

    toplevel->commit.notify = handle_commit;
    wl_signal_add(&wlr_toplevel->base->surface->events.commit, &toplevel->commit);

    toplevel->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_toplevel->events.destroy, &toplevel->destroy);

    toplevel->request_move.notify = handle_request_move;
    wl_signal_add(&wlr_toplevel->events.request_move, &toplevel->request_move);

    toplevel->request_resize.notify = handle_request_resize;
    wl_signal_add(&wlr_toplevel->events.request_resize, &toplevel->request_resize);

    toplevel->request_maximize.notify = handle_request_maximize;
    wl_signal_add(&wlr_toplevel->events.request_maximize, &toplevel->request_maximize);

    toplevel->request_fullscreen.notify = handle_request_fullscreen;
    wl_signal_add(&wlr_toplevel->events.request_fullscreen, &toplevel->request_fullscreen);

    toplevel->set_app_id.notify = handle_set_app_id;
    wl_signal_add(&wlr_toplevel->events.set_app_id, &toplevel->set_app_id);

    toplevel->set_title.notify = handle_set_title;
    wl_signal_add(&wlr_toplevel->events.set_title, &toplevel->set_title);

    return toplevel;
}

void
toplevel_float_raise(struct state *state, struct toplevel *toplevel) {
    UNUSED(state);

    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    // move to top
    wl_list_remove(&toplevel->link);
    wl_list_insert(&toplevel->workspace->floats, &toplevel->link);

    wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
}

void
toplevel_focus(struct state *state, struct toplevel *toplevel) {
    if(state->lock_mgr.lock || state->focused_lock || state->is_exclusive ||
            (state->grabbed_toplevel && toplevel != state->grabbed_toplevel) ||
            (toplevel && toplevel->workspace->fullscreen && toplevel != toplevel->workspace->fullscreen)) {
        return;
    }

    struct toplevel *prev = state->focused_toplevel;
    if(prev == toplevel) {
        // already focused
        return;
    }

    if(prev) {
        // unfocus it
        wlr_xdg_toplevel_set_activated(prev->wlr_toplevel, false);
        wlr_foreign_toplevel_handle_v1_set_activated(prev->foreign_toplevel_handle, false);
        toplevel_set_border_color(prev, state->config.border.color.inactive);
    }

    if(!toplevel) {
        // this means we just wanted to unfocus whatever was focused
        return;
    }

    state->focused_toplevel = toplevel;

    wlr_xdg_toplevel_set_activated(toplevel->wlr_toplevel, true);
    wlr_foreign_toplevel_handle_v1_set_activated(toplevel->foreign_toplevel_handle, true);
    toplevel_set_border_color(toplevel, state->config.border.color.active);

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(state->seat.wlr_seat);
    if(keyboard) {
        wlr_seat_keyboard_notify_enter(state->seat.wlr_seat, toplevel->wlr_toplevel->base->surface, keyboard->keycodes,
                keyboard->num_keycodes, &keyboard->modifiers);
    }
}

void
toplevel_move_to_workspace(struct state *state, struct toplevel *toplevel, struct workspace *workspace) {
    // TODO
}

u32
toplevel_get_corner_closest_to(struct toplevel *toplevel, int x, int y) {
    int left_dist = x - toplevel->current.x;
    int right_dist = toplevel->current.width - left_dist;
    int top_dist = y - toplevel->current.y;
    int bottom_dist = toplevel->current.height - top_dist;

    u32 edges = 0;
    if(left_dist <= right_dist) {
        edges |= WLR_EDGE_LEFT;
    } else {
        edges |= WLR_EDGE_RIGHT;
    }

    if(top_dist <= bottom_dist) {
        edges |= WLR_EDGE_TOP;
    } else {
        edges |= WLR_EDGE_BOTTOM;
    }

    return edges;
}

void
toplevel_set_border_color(struct toplevel *toplevel, color_t color) {
    float wlr_color[4];
    color_to_wlr_color(color, wlr_color);
    wlr_scene_rect_set_color(toplevel->border, wlr_color);
}

static void
set_fullscreen(struct state *state, struct toplevel *toplevel) {
    if(!toplevel->wlr_toplevel->base->surface->mapped || toplevel->state == TOPLEVEL_STATE_FULLSCREEN ||
            toplevel->workspace->fullscreen || toplevel == state->grabbed_toplevel) {
        return;
    }

    struct workspace *workspace = toplevel->workspace;
    struct output *output = workspace->output;

    toplevel->prev_geometry = toplevel->current;

    workspace->fullscreen = toplevel;
    toplevel->state = TOPLEVEL_STATE_FULLSCREEN;

    wlr_xdg_toplevel_set_fullscreen(toplevel->wlr_toplevel, true);
    toplevel_configure(state, toplevel, &output->full_area);
    wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.fullscreen);

    wlr_foreign_toplevel_handle_v1_set_fullscreen(toplevel->foreign_toplevel_handle, true);
}

static void
unset_fullscreen(struct state *state, struct toplevel *toplevel) {
    if(toplevel->state != TOPLEVEL_STATE_FULLSCREEN) {
        return;
    }

    struct workspace *workspace = toplevel->workspace;
    workspace->fullscreen = NULL;

    wlr_xdg_toplevel_set_fullscreen(toplevel->wlr_toplevel, false);

    toplevel->state = toplevel->prev_state;
    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        toplevel_configure(state, toplevel, &toplevel->prev_geometry);
        wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.floats);
    } else {
        wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.tiled);
    }

    layout_configure(state, workspace);
    wlr_foreign_toplevel_handle_v1_set_fullscreen(toplevel->foreign_toplevel_handle, false);
}

void
toplevel_set_fullscreen(struct state *state, struct toplevel *toplevel, bool set) {
    if(set) {
        set_fullscreen(state, toplevel);
    } else {
        unset_fullscreen(state, toplevel);
    }
}

struct output *
toplevel_float_largest_output_intersection(struct state *state, struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    int max_area = 0;
    struct output *output = NULL;

    struct output *iter;
    wl_list_for_each(iter, &state->outputs, link) {
        struct wlr_box intersection_box;
        bool intersects = wlr_box_intersection(&intersection_box, &toplevel->current, &iter->full_area);
        if(intersects) {
            int area = wlr_box_area(&intersection_box);
            if(area > max_area) {
                max_area = area;
                output = iter;
            }
        }
    }

    return output;
}

void
toplevel_configure(struct state *state, struct toplevel *toplevel, struct wlr_box *box) {
    toplevel->pending = *box;

    if(toplevel->current.width == toplevel->pending.width && toplevel->current.height == toplevel->pending.height) {
        // since we might be in the middle of `layout_configure()` here, we dont want to commit it right
        // away, as we need to wait for all other toplevels to be informed about its new state. hence, we
        // delay the commit in the event loop, meaning its going to be performed after the current event is
        // processed at least.
        transaction_schedule_commit(state, toplevel);
        return;
    };

    toplevel->configure_serial = wlr_xdg_toplevel_set_size(toplevel->wlr_toplevel, box->width, box->height);
    transaction_mark_dirty(state, toplevel);
}

// void
// toplevel_recheck_opacity_rules(struct mwc_toplevel *toplevel) {
//   /* check if it satisfies some window rule */
//   struct window_rule_opacity *w;
//   bool set = false;
//   wl_list_for_each(w, &server.config->window_rules.opacity, link) {
//     if(toplevel_matches_window_rule(toplevel, &w->condition)) {
//       toplevel->inactive_opacity = w->inactive_value;
//       toplevel->active_opacity = w->active_value;
//       set = true;
//       break;
//     }
//   }
//
//   if(!set) {
//     toplevel->inactive_opacity = server.config->inactive_opacity;
//     toplevel->active_opacity = server.config->active_opacity;
//   }
// }
//
// bool
// toplevel_matches_window_rule(struct mwc_toplevel *toplevel,
//                              struct window_rule_regex *condition) {
//   char *app_id = toplevel->xdg_toplevel->app_id;
//   char *title = toplevel->xdg_toplevel->title;
//
//   bool matches_app_id;
//   if(condition->has_app_id_regex) {
//     if(app_id == NULL) {
//       matches_app_id = false;
//     } else {
//       matches_app_id = regexec(&condition->app_id_regex, app_id, 0, NULL, 0) == 0;
//     }
//   } else {
//     matches_app_id = true;
//   }
//
//   bool matches_title;
//   if(condition->has_title_regex) {
//     if(title == NULL) {
//       matches_title = false;
//     } else {
//       matches_title = regexec(&condition->title_regex, title, 0, NULL, 0) == 0;
//     }
//   } else {
//     matches_title = true;
//   }
//
//   return matches_app_id && matches_title;
// }
//
// void
// toplevel_floating_size(struct mwc_toplevel *toplevel, uint32_t *width, uint32_t *height) {
//   struct window_rule_size *w;
//   wl_list_for_each(w, &server.config->window_rules.size, link) {
//     if(toplevel_matches_window_rule(toplevel, &w->condition)) {
//       if(w->relative_width) {
//         *width = toplevel->workspace->output->usable_area.width * w->width / 100;
//       } else {
//         *width = w->width;
//       }
//
//       if(w->relative_height) {
//         *height = toplevel->workspace->output->usable_area.height * w->height / 100;
//       } else {
//         *height = w->height;
//       }
//
//       return;
//     }
//   }
//
//   *width = 0;
//   *height = 0;
// }
//
// void
// cursor_jump_focused_toplevel(void) {
//   struct mwc_toplevel *toplevel = server.focused_toplevel;
//   if(toplevel == NULL) return;
//
//   struct wlr_box geo_box = toplevel_get_geometry(toplevel);
//   wlr_cursor_warp(server.cursor, NULL,
//                   toplevel->scene_tree->node.x + geo_box.x + toplevel->current.width / 2.0,
//                   toplevel->scene_tree->node.y + geo_box.y + toplevel->current.height / 2.0);
//
//   struct timespec now;
//   clock_gettime(CLOCK_MONOTONIC, &now);
//
//   pointer_handle_focus(now.tv_sec * 1000 + now.tv_nsec / 1000, false);
// }
//
// void
// toplevel_set_pending_state(struct mwc_toplevel *toplevel, uint32_t x, uint32_t y,
//                            uint32_t width, uint32_t height) {
//   struct wlr_box pending = {
//     .x = x,
//     .y = y,
//     .width = width,
//     .height = height,
//   };
//
//   toplevel->pending = pending;
//
//   if(!server.config->animations || toplevel == server.grabbed_toplevel
//      || wlr_box_equal(&toplevel->current, &pending)) {
//     toplevel->animation.should_animate = false;
//   } else {
//     toplevel->animation.should_animate = true;
//     toplevel->animation.initial = toplevel->current;
//   }
//
//   if(toplevel->current.width == toplevel->pending.width
//      && toplevel->current.height == toplevel->pending.height) {
//     toplevel_commit(toplevel);
//     return;
//   };
//
//   toplevel->configure_serial = wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel,
//                                                          width, height);
//   toplevel->dirty = true;
// }
//
// void
// toplevel_move(void) {
//   /* move the grabbed toplevel to the new position */
//   struct mwc_toplevel *toplevel = server.grabbed_toplevel;
//
//   int32_t new_x = server.grabbed_toplevel_initial_box.x + (server.cursor->x - server.grab_x);
//   int32_t new_y = server.grabbed_toplevel_initial_box.y + (server.cursor->y - server.grab_y);
//
//   toplevel_set_pending_state(toplevel, new_x, new_y,
//                              toplevel->current.width, toplevel->current.height);
// }
//
// void
// toplevel_resize(void) {
//   struct mwc_toplevel *toplevel = server.grabbed_toplevel;
//
//   toplevel->resizing = true;
//
//   int start_x = server.grabbed_toplevel_initial_box.x;
//   int start_y = server.grabbed_toplevel_initial_box.y;
//   int start_width = server.grabbed_toplevel_initial_box.width;
//   int start_height = server.grabbed_toplevel_initial_box.height;
//
//   int new_x = server.grabbed_toplevel_initial_box.x;
//   int new_y = server.grabbed_toplevel_initial_box.y;
//   int new_width = server.grabbed_toplevel_initial_box.width;
//   int new_height = server.grabbed_toplevel_initial_box.height;
//
//   int min_width = max(toplevel->xdg_toplevel->current.min_width,
//                       server.config->min_toplevel_size);
//   int min_height = max(toplevel->xdg_toplevel->current.min_height,
//                        server.config->min_toplevel_size);
//
//   if(server.resize_edges & WLR_EDGE_TOP) {
//     new_y = start_y + (server.cursor->y - server.grab_y);
//     new_height = start_height - (server.cursor->y - server.grab_y);
//     if(new_height <= min_height) {
//       new_y = start_y + start_height - min_height;
//       new_height = min_height;
//     }
//   } else if(server.resize_edges & WLR_EDGE_BOTTOM) {
//     new_y = start_y;
//     new_height = start_height + (server.cursor->y - server.grab_y);
//     if(new_height <= min_height) {
//       new_height = min_height;
//     }
//   }
//   if(server.resize_edges & WLR_EDGE_LEFT) {
//     new_x = start_x + (server.cursor->x - server.grab_x);
//     new_width = start_width - (server.cursor->x - server.grab_x);
//     if(new_width <= min_width) {
//       new_x = start_x + start_width - min_width;
//       new_width = min_width;
//     }
//   } else if(server.resize_edges & WLR_EDGE_RIGHT) {
//     new_x = start_x;
//     new_width = start_width + (server.cursor->x - server.grab_x);
//     if(new_width <= min_width) {
//       new_width = min_width;
//     }
//   }
//
//   toplevel_set_pending_state(toplevel, new_x, new_y, new_width, new_height);
// }
//
// void
// unfocus_focused_toplevel(void) {
//   struct mwc_toplevel *toplevel = server.focused_toplevel;
//   if(toplevel == NULL) return;
//
//   server.focused_toplevel = NULL;
//   /* deactivate the surface */
//   wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, false);
//   /* clear all focus on the keyboard, focusing new should set new toplevel focus */
//   wlr_seat_keyboard_clear_focus(server.seat);
//   wlr_seat_pointer_clear_focus(server.seat);
//
//   ipc_broadcast_message(IPC_ACTIVE_TOPLEVEL);
//   wlr_foreign_toplevel_handle_v1_set_activated(toplevel->foreign_toplevel_handle, false);
//
//   /* we schedule a frame in order for borders to be redrawn */
//   wlr_output_schedule_frame(toplevel->workspace->output->wlr_output);
// }
//
// void
// focus_toplevel(struct mwc_toplevel *toplevel) {
//   /* there has been an issue with some electron apps that do not
//    * want to map the surface, and neither want to destroy themselfs */
//   if(server.lock != NULL) return;
//   if(server.exclusive) return;
//   if(server.grabbed_toplevel != NULL) return;
//   if(toplevel->workspace->fullscreen_toplevel != NULL
//      && toplevel != toplevel->workspace->fullscreen_toplevel) return;
//
//   struct mwc_toplevel *prev_toplevel = server.focused_toplevel;
//   if(prev_toplevel == toplevel) return;
//
//   if(prev_toplevel != NULL) {
//     wlr_xdg_toplevel_set_activated(prev_toplevel->xdg_toplevel, false);
//     wlr_foreign_toplevel_handle_v1_set_activated(toplevel->foreign_toplevel_handle, false);
//   }
//
//   server.focused_toplevel = toplevel;
//
//   if(toplevel->floating) {
//     wl_list_remove(&toplevel->link);
//     wl_list_insert(&toplevel->workspace->floating_toplevels, &toplevel->link);
//   }
//
// 	wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);
//   wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
//
//   struct wlr_seat *seat = server.seat;
//   struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
//   if(keyboard != NULL) {
//     wlr_seat_keyboard_notify_enter(seat, toplevel->xdg_toplevel->base->surface,
//                                    keyboard->keycodes, keyboard->num_keycodes,
//                                    &keyboard->modifiers);
//   }
//
//   ipc_broadcast_message(IPC_ACTIVE_TOPLEVEL);
//   wlr_foreign_toplevel_handle_v1_set_activated(toplevel->foreign_toplevel_handle, true);
//
//   /* we schedule a frame in order for borders to be redrawn */
//   wlr_output_schedule_frame(toplevel->workspace->output->wlr_output);
// }
//
//
// struct mwc_toplevel *
// toplevel_find_closest_floating_on_workspace(struct mwc_toplevel *toplevel,
//                                             enum mwc_direction direction) {
//   assert(toplevel->floating);
//   struct mwc_workspace *workspace = toplevel->workspace;
//
//   struct mwc_toplevel *min = NULL;
//   uint32_t min_val = UINT32_MAX;
//
//   struct mwc_toplevel *t;
//   switch(direction) {
//     case MWC_UP: {
//       wl_list_for_each(t, &workspace->floating_toplevels, link) {
//         if(t == toplevel || Y(t) > Y(toplevel)) continue;
//
//         uint32_t dy = abs((int)Y(toplevel) - Y(t));
//         if(dy < min_val) {
//           min = t;
//           min_val = dy;
//         }
//       }
//       return min;
//     }
//     case MWC_DOWN: {
//       wl_list_for_each(t, &workspace->floating_toplevels, link) {
//         if(t == toplevel || Y(t) < Y(toplevel)) continue;
//
//         uint32_t dy = abs((int)Y(toplevel) - Y(t));
//         if(dy < min_val) {
//           min = t;
//           min_val = dy;
//         }
//       }
//       return min;
//     }
//     case MWC_LEFT: {
//       wl_list_for_each(t, &workspace->floating_toplevels, link) {
//         if(t == toplevel || X(t) > X(toplevel)) continue;
//
//         uint32_t dx = abs((int)X(toplevel) - X(t));
//         if(dx < min_val) {
//           min = t;
//           min_val = dx;
//         }
//       }
//       return min;
//     }
//     case MWC_RIGHT: {
//       wl_list_for_each(t, &workspace->floating_toplevels, link) {
//         if(t == toplevel || X(t) < X(toplevel)) continue;
//
//         uint32_t dx = abs((int)X(toplevel) - X(t));
//         if(dx < min_val) {
//           min = t;
//           min_val = dx;
//         }
//       }
//       return min;
//     }
//   }
// }
//
// struct mwc_output *
// toplevel_get_primary_output(struct mwc_toplevel *toplevel) {
//   struct wlr_box intersection_box;
//   struct wlr_box output_box;
//   uint32_t max_area = 0;
//   struct mwc_output *max_area_output = NULL;
//
//   struct mwc_output *o;
//   wl_list_for_each(o, &server.outputs, link) {
//     wlr_output_layout_get_box(server.output_layout, o->wlr_output, &output_box);
//     bool intersects =
//       wlr_box_intersection(&intersection_box, &toplevel->current, &output_box);
//     if(intersects && box_area(&intersection_box) > max_area) {
//       max_area = box_area(&intersection_box);
//       max_area_output = o;
//     }
//   }
//
//   return max_area_output;
// }
//
//
// void
// toplevel_tiled_insert_into_layout(struct mwc_toplevel *toplevel, uint32_t x, uint32_t y) {
//   struct mwc_workspace *workspace = server.active_workspace;
//
//   toplevel->workspace = workspace;
//
//   struct mwc_toplevel *under_cursor = layout_toplevel_at(workspace, x, y);
//
//   if(under_cursor == NULL) {
//     if(wl_list_length(&workspace->masters) < server.config->master_count) {
//       wl_list_insert(workspace->masters.prev, &toplevel->link);
//     } else {
//       wl_list_insert(workspace->slaves.prev, &toplevel->link);
//     }
//   } else {
//     bool on_left_side = x <= under_cursor->current.x + under_cursor->current.width / 2;
//     bool on_top_side = y <= under_cursor->current.y + under_cursor->current.height / 2;
//     bool under_cursor_is_master = toplevel_is_master(under_cursor);
//
//     /* we insert it before under_cursor if either:
//      *   - its last master and there are some slaves
//      *   - cursor is on left (top) */
//     if((under_cursor_is_master && &under_cursor->link == workspace->masters.prev
//        && wl_list_length(&workspace->slaves) > 0)
//        || (under_cursor_is_master && on_left_side)
//        || (!under_cursor_is_master && on_top_side)) {
//       wl_list_insert(under_cursor->link.prev, &toplevel->link);
//     } else {
//       wl_list_insert(&under_cursor->link, &toplevel->link);
//     }
//
//     if(wl_list_length(&workspace->masters) > server.config->master_count) {
//       struct mwc_toplevel *last = wl_container_of(workspace->masters.prev, last, link);
//       wl_list_remove(&last->link);
//       wl_list_insert(workspace->slaves.prev, &last->link);
//     }
//   }
// }
//
// void
// xdg_activation_handle_token_destroy(struct wl_listener *listener, void *data) {
// 	struct mwc_token *token_data = wl_container_of(listener, token_data, destroy);
// 	wl_list_remove(&token_data->destroy.link);
//
// 	free(token_data);
// }
//
// void
// xdg_activation_handle_new_token(struct wl_listener *listener, void *data) {
// 	struct wlr_xdg_activation_token_v1 *wlr_token = data;
//   if(wlr_token->surface == NULL || wlr_token->seat == NULL) return;
//
// 	struct mwc_token *token = calloc(1, sizeof(*token));
//   token->wlr_token = wlr_token;
// 	wlr_token->data = token;
//
// 	token->destroy.notify = xdg_activation_handle_token_destroy;
// 	wl_signal_add(&wlr_token->events.destroy, &token->destroy);
// }
//
// void
// xdg_activation_handle_request(struct wl_listener *listener, void *data) {
// 	const struct wlr_xdg_activation_v1_request_activate_event *event = data;
//
// 	struct wlr_xdg_surface *xdg_surface = wlr_xdg_surface_try_from_wlr_surface(event->surface);
// 	if(xdg_surface == NULL) return;
//
// 	struct wlr_scene_tree *tree = xdg_surface->data;
//   /* this happens if the toplevel has not been mapped yet. anyway it does not make sense to
//    * request that i activate this surface that is not on the screen */
//   if(tree == NULL) return;
//
//   struct mwc_something *something = tree->node.data;
//   if(something == NULL) return;
//
//   if(something->type == MWC_POPUP) {
//     something = popup_get_root_parent(something->popup);
//   }
//
//   if(something->type != MWC_TOPLEVEL) return;
//
//   struct mwc_toplevel *toplevel = something->toplevel;
//
//   focus_toplevel(toplevel);
// }
