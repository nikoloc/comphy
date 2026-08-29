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
#include "rules.h"
#include "transaction.h"
#include "util/macros.h"
#include "util/memory.h"
#include "util/time_util.h"
#include "workspace.h"

static void
set_hacks(struct toplevel *toplevel) {
    // in order for the clients to react better to our demands in tiled mode, we tell it that its maximized and
    // tiled on all sides. this makes the client not draw stuff such as rounded corners, shadows and usually
    // means the client will respect our desired size more than if not
    wlr_xdg_toplevel_set_maximized(toplevel->wlr_toplevel, true);
    wlr_xdg_toplevel_set_tiled(toplevel->wlr_toplevel, WLR_EDGE_TOP | WLR_EDGE_RIGHT | WLR_EDGE_BOTTOM | WLR_EDGE_LEFT);
}

static void
reset_hacks(struct toplevel *toplevel) {
    wlr_xdg_toplevel_set_maximized(toplevel->wlr_toplevel, false);
    wlr_xdg_toplevel_set_tiled(toplevel->wlr_toplevel, WLR_EDGE_NONE);
}

static bool
matches_rule(struct toplevel *toplevel, struct toplevel_rule *rule) {
    if((rule->fields & TOPLEVEL_RULE_FIELD_MATCH_APP_ID) &&
            (!toplevel->wlr_toplevel->app_id || !strstr(toplevel->wlr_toplevel->app_id, rule->match.app_id))) {
        return false;
    }

    if((rule->fields & TOPLEVEL_RULE_FIELD_MATCH_TITLE) &&
            (!toplevel->wlr_toplevel->title || !strstr(toplevel->wlr_toplevel->title, rule->match.title))) {
        return false;
    }

    return true;
}

static inline bool
should_float(struct toplevel *toplevel) {
    // we make toplevels float if they have fixed size or are children of another toplevel
    return (toplevel->wlr_toplevel->current.max_height > 0 &&
                   toplevel->wlr_toplevel->current.max_height == toplevel->wlr_toplevel->current.min_height) ||
           (toplevel->wlr_toplevel->current.max_width > 0 &&
                   toplevel->wlr_toplevel->current.max_width == toplevel->wlr_toplevel->current.min_width) ||
           toplevel->wlr_toplevel->parent;
}

static enum toplevel_state
default_state(struct state *state, struct toplevel *toplevel) {
    struct toplevel_rule *iter;
    wl_list_for_each(iter, &state->config.toplevel_rules, link) {
        if((iter->fields & TOPLEVEL_RULE_FIELD_STATE) && matches_rule(toplevel, iter)) {
            return iter->state;
        }
    }

    if(should_float(toplevel)) {
        return TOPLEVEL_STATE_FLOAT;
    }

    return TOPLEVEL_STATE_TILED;
}

static void
default_size(struct state *state, struct toplevel *toplevel, int *width, int *height) {
    *width = 0, *height = 0;

    struct toplevel_rule *iter;
    wl_list_for_each(iter, &state->config.toplevel_rules, link) {
        if((iter->fields & TOPLEVEL_RULE_FIELD_WIDTH) && matches_rule(toplevel, iter)) {
            *width = iter->width;
        }

        if((iter->fields & TOPLEVEL_RULE_FIELD_HEIGHT) && matches_rule(toplevel, iter)) {
            *height = iter->height;
        }
    }
}

static bool
should_have_border(struct state *state, struct toplevel *toplevel) {
    bool smart_gaps = state->config.gaps.smart && toplevel == toplevel->workspace->master &&
                      wl_list_empty(&toplevel->workspace->slaves);

    return toplevel->state != TOPLEVEL_STATE_FULLSCREEN && !smart_gaps;
}

static void
center_float(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    struct wlr_box output_box = toplevel->workspace->output->usable_area;
    toplevel->pending.x = output_box.x + (output_box.width - toplevel->pending.width) / 2;
    toplevel->pending.y = output_box.y + (output_box.height - toplevel->pending.height) / 2;

    toplevel->needs_centering = false;
}

static void
raise_children_above(struct toplevel *toplevel) {
    struct toplevel *iter;
    wl_list_for_each(iter, &toplevel->workspace->floats, link) {
        if(iter->wlr_toplevel->parent == toplevel->wlr_toplevel) {
            // if its a child of this toplevel we raise it above this one, which will recursively raise all of its
            // children above itself
            wlr_scene_node_place_above(&iter->scene_tree->node, &toplevel->scene_tree->node);
            raise_children_above(iter);
        }
    }
}

static void
raise_parent_just_bellow_if_any(struct toplevel *toplevel) {
    if(toplevel->wlr_toplevel->parent == NULL) {
        return;
    }

    struct toplevel *parent = toplevel->wlr_toplevel->parent->base->data;
    if(parent->state == TOPLEVEL_STATE_FLOAT) {
        // we raise this one and its parent (if any)
        wlr_scene_node_place_below(&parent->scene_tree->node, &toplevel->scene_tree->node);
        raise_parent_just_bellow_if_any(parent);
    }
}

void
toplevel_handle_parents_and_children(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    // if floating we raise its parent (and parents parent and so on)
    raise_parent_just_bellow_if_any(toplevel);
    // and also raise its children above this one
    raise_children_above(toplevel);
}

void
toplevel_raise(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    // keep it first
    wl_list_remove(&toplevel->link);
    wl_list_insert(&toplevel->workspace->floats, &toplevel->link);

    wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
    toplevel_handle_parents_and_children(toplevel);
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

    float color[4];
    color_to_wlr_color(state->config.border.color.inactive, color);
    toplevel->border = wlr_scene_rect_create(toplevel->scene_tree, 0, 0, color);
    wlr_scene_node_lower_to_bottom(&toplevel->border->node);

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
            wl_list_insert(&workspace->floats, &toplevel->link);
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

    toplevel_focus(state, toplevel, true);
}

static struct toplevel *
find_next_to_focus_float(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    struct wl_list *next = wl_list_next_or_prev(&toplevel->workspace->floats, &toplevel->link);
    if(next) {
        return CONTAINER_OF(next, struct toplevel, link);
    }

    // might be NULL
    return toplevel->workspace->master;
}

static struct toplevel *
find_next_to_focus_tiled(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_TILED);

    if(toplevel == toplevel->workspace->master) {
        struct wl_list *bottom_slave = wl_list_last(&toplevel->workspace->slaves);
        if(bottom_slave) {
            return CONTAINER_OF(bottom_slave, struct toplevel, link);
        }
    } else {
        // for slaves first try other slaves, then master
        struct wl_list *next = wl_list_next_or_prev(&toplevel->workspace->slaves, &toplevel->link);
        if(next) {
            return CONTAINER_OF(next, struct toplevel, link);
        }

        if(toplevel->workspace->master) {
            return toplevel->workspace->master;
        }
    }

    // if none, then try floats
    struct wl_list *next = wl_list_first(&toplevel->workspace->floats);
    if(next) {
        return CONTAINER_OF(next, struct toplevel, link);
    }

    return NULL;
}

static void
handle_unmap(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, unmap);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "toplevel '%p' unmapped", (void *)toplevel);

    struct workspace *workspace = toplevel->workspace;

    if(toplevel == state->warp_on_transaction) {
        state->warp_on_transaction = NULL;
    }

    if(toplevel == state->grabbed_toplevel) {
        operation_stop_whatever(state);
        // in this case `toplevel->workspace` may not reflect valid state, since we only update it once the
        // toplevel is dropped, so we use `state->active_workspace`
        output_focus(state, state->active_workspace->output, true);
    } else {
        switch(toplevel->state) {
            case TOPLEVEL_STATE_TILED: {
                if(toplevel == state->focused_toplevel) {
                    // before removing the toplevel from the layout etc, find the one to give the focus next
                    struct toplevel *focus_next = find_next_to_focus_tiled(toplevel);
                    toplevel_focus(state, focus_next, false);
                }

                layout_remove(toplevel);
                layout_configure(state, toplevel->workspace);
                break;
            }
            case TOPLEVEL_STATE_FLOAT: {
                if(toplevel == state->focused_toplevel) {
                    // before removing the toplevel from the layout etc, find the one to give the focus next
                    struct toplevel *focus_next = find_next_to_focus_float(toplevel);
                    toplevel_focus(state, focus_next, false);
                }

                wl_list_remove(&toplevel->link);
                break;
            }
            case TOPLEVEL_STATE_FULLSCREEN: {
                workspace->fullscreen = NULL;
                output_focus(state, workspace->output, true);
                break;
            }
        }
    }

    // add it as a ghost for the next transaction
    toplevel->is_ghost = true;
    wl_list_insert(&workspace->ghosts, &toplevel->link);
    wlr_scene_node_set_enabled(&toplevel->scene_tree->node, true);
    transaction_mark_dirty(state, toplevel);
}

static void
send_frame_done_iter(struct wlr_scene_node *node, struct timespec *now) {
    switch(node->type) {
        case WLR_SCENE_NODE_TREE: {
            struct wlr_scene_tree *tree = wlr_scene_tree_from_node(node);
            wl_list_for_each(node, &tree->children, link) {
                send_frame_done_iter(node, now);
            }
            break;
        }
        case WLR_SCENE_NODE_RECT: {
            break;
        }
        case WLR_SCENE_NODE_BUFFER: {
            struct wlr_scene_buffer *buffer = wlr_scene_buffer_from_node(node);
            struct wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(buffer);
            if(scene_surface) {
                wlr_surface_send_frame_done(scene_surface->surface, now);
            }
            break;
        }
    }
}

void
toplevel_send_frame_done(struct toplevel *toplevel) {
    struct timespec now = time_now_timespec();
    send_frame_done_iter(&toplevel->scene_tree->node, &now);
}

static void
send_scale(struct toplevel *toplevel, float scale) {
    wlr_fractional_scale_v1_notify_scale(toplevel->wlr_toplevel->base->surface, scale);
    // fallback for the clients not supporting the fractional scale protocol
    wlr_surface_set_preferred_buffer_scale(toplevel->wlr_toplevel->base->surface, ceil(scale));
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
            default_size(state, toplevel, &width, &height);
            toplevel->needs_centering = true;
        }

        if(state->active_workspace) {
            // if there is an output we try and guess this is going to be the output this toplevel is going to be
            // displayed on. this might change if the user changes the workspace for example, or for any other reason,
            // but is a good guess most of the time. we send this outputs scale info to the client, so it can map the
            // surface with that scale in mind, resulting in less flicker
            struct output *current_output = state->active_workspace->output;
            send_scale(toplevel, current_output->wlr_output->scale);
        }

        wlr_xdg_toplevel_set_size(toplevel->wlr_toplevel, width, height);
        wlr_xdg_toplevel_set_wm_capabilities(toplevel->wlr_toplevel, WLR_XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN);
        if(toplevel->state == TOPLEVEL_STATE_TILED) {
            // send hacks
        }
        return;
    }

    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        // we conform to the size it chose, even if its not the one we requested, or we did not request it at all
        struct wlr_box *geometry = &toplevel->wlr_toplevel->base->geometry;

        wlr_log(WLR_DEBUG, "toplevel '%p' commited with size %dx%d", (void *)toplevel, geometry->width,
                geometry->height);

        toplevel->pending.width = geometry->width;
        toplevel->pending.height = geometry->height;

        // need to add the border size to the box
        if(toplevel->has_border) {
            toplevel->pending.width += 2 * state->config.border.width;
            toplevel->pending.height += 2 * state->config.border.width;
        }

        if(toplevel->needs_centering) {
            center_float(toplevel);
        }

        transaction_commit(state, toplevel);
        return;
    }

    if(toplevel->transaction_state != TRANSACTION_STATE_DIRTY) {
        wlr_log(WLR_DEBUG, "toplevel commited but not dirty");
        return;
    }

    u32 serial = toplevel->wlr_toplevel->base->current.configure_serial;
    if(serial < toplevel->configure_serial) {
        wlr_log(WLR_DEBUG, "toplevel commited but old serial");
        // send a frame event, this makes the client commit a new buffer, conforming to our new state, in order to
        // conform to our transaction state. kinda hacky, but thats just how clients operate under wayland
        toplevel_send_frame_done(toplevel);
        return;
    }

    // commit the new state for this toplevel. this will check for all the other toplevels on the screen and finalize
    // the state for the output if everything is perfect, else its going to wait for others
    transaction_commit(state, toplevel);
}

void
toplevel_finalize_destroy(struct toplevel *toplevel) {
    // we need to manually destroy it since we manually created it. NOTE: this is also going to destroy the snapshot
    // tree if it exists
    wlr_scene_node_destroy(&toplevel->scene_tree->node);
    FREE(toplevel);
}

static void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, destroy);
    ASSERT(!toplevel->wlr_toplevel->base->surface->mapped);

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

    if(toplevel->is_ghost) {
        // dont destroy it fully, but keep the presentation and the pointer valid and flag it
        toplevel->is_destroyed = true;
        return;
    }

    toplevel_finalize_destroy(toplevel);
}

static void
handle_request_move(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, request_move);
    struct state *state = state_get();

    if(!toplevel->wlr_toplevel->base->surface->mapped) {
        return;
    }

    enum view *pointer_focused = seat_get_pointer_focused(state);
    if(!pointer_focused || toplevel != view_get_toplevel(pointer_focused)) {
        return;
    }

    operation_start_move(state, toplevel, false);
}

static void
handle_request_resize(struct wl_listener *listener, void *data) {
    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    struct state *state = state_get();

    if(!toplevel->wlr_toplevel->base->surface->mapped) {
        return;
    }

    enum view *pointer_focused = seat_get_pointer_focused(state);
    if(!pointer_focused || toplevel != view_get_toplevel(pointer_focused)) {
        return;
    }

    operation_start_resize(state, toplevel, event->edges, false);
}

static void
handle_request_maximize(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct toplevel *toplevel = CONTAINER_OF(listener, struct toplevel, request_maximize);

    // no op
    if(toplevel->wlr_toplevel->base->initialized) {
        // wlr_xdg_surface_schedule_configure(toplevel->wlr_toplevel->base);
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
        toplevel_set_state(state, toplevel, TOPLEVEL_STATE_FULLSCREEN);
    } else {
        toplevel_set_state(state, toplevel, toplevel->prev_state);
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

struct toplevel *
toplevel_create(struct state *state, struct wlr_xdg_toplevel *wlr_toplevel) {
    struct toplevel *toplevel = ALLOC(struct toplevel);
    toplevel->wlr_toplevel = wlr_toplevel;
    wlr_toplevel->base->data = toplevel;

    toplevel->view = VIEW_TOPLEVEL;
    // we create the tree in the tiled tree, and swap it later if necessery
    toplevel->scene_tree = wlr_scene_tree_create(state->scene.trees.tiled);
    // in order to obtain this toplevel we keep a pointer to view, from which the type of view can be read, and then
    // extracted by using `CONTAINER_OF()`
    toplevel->scene_tree->node.data = &toplevel->view;

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
toplevel_focus(struct state *state, struct toplevel *toplevel, bool warp) {
    if(state->lock_mgr.lock || state->focused_lock || state->is_exclusive ||
            (state->grabbed_toplevel && toplevel != state->grabbed_toplevel) ||
            (toplevel && toplevel->workspace->fullscreen && toplevel != toplevel->workspace->fullscreen)) {
        return;
    }

    // TODO: instead add a view_unfocus_current() that should be called in every focus request
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

    state->focused_toplevel = toplevel;

    if(!toplevel) {
        // this means we just wanted to unfocus whatever was focused
        return;
    }

    wlr_xdg_toplevel_set_activated(toplevel->wlr_toplevel, true);
    wlr_foreign_toplevel_handle_v1_set_activated(toplevel->foreign_toplevel_handle, true);
    toplevel_set_border_color(toplevel, state->config.border.color.active);

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(state->seat.wlr_seat);
    if(keyboard) {
        wlr_seat_keyboard_notify_enter(state->seat.wlr_seat, toplevel->wlr_toplevel->base->surface, keyboard->keycodes,
                keyboard->num_keycodes, &keyboard->modifiers);
    }

    if(warp && state->config.cursor.warp) {
        if(toplevel->transaction_state == TRANSACTION_STATE_CLEAN) {
            cursor_warp_toplevel(state, toplevel);
        } else {
            // mark it so when the transaction commits the cursor is warped
            state->warp_on_transaction = toplevel;
            transaction_schedule_commit(state, toplevel->workspace);
        }
    }
}

void
toplevel_move_to_workspace(struct state *state, struct toplevel *toplevel, struct workspace *workspace) {
    if(toplevel == state->grabbed_toplevel || toplevel->workspace == workspace) {
        return;
    }

    struct workspace *old_workspace = toplevel->workspace;

    switch(toplevel->state) {
        case TOPLEVEL_STATE_TILED: {
            layout_remove(toplevel);
            layout_add(workspace, toplevel);

            // we need to only now update the workspace, so the transactions reference the real workspace
            toplevel->workspace = workspace;

            // configuring layouts now has all the information updated
            layout_configure(state, old_workspace);
            layout_configure(state, workspace);
            break;
        }
        case TOPLEVEL_STATE_FLOAT: {
            toplevel->workspace = workspace;

            wl_list_remove(&toplevel->link);
            wl_list_insert(&workspace->floats, &toplevel->link);

            if(old_workspace->output != workspace->output) {
                // center it on the new output
                struct wlr_box *output_box = &workspace->output->usable_area;
                struct wlr_box box = {
                        .x = output_box->x + (output_box->width - toplevel->current.width) / 2,
                        .y = output_box->y + (output_box->height - toplevel->current.height) / 2,
                        .width = toplevel->current.width,
                        .height = toplevel->current.height,
                };
                toplevel_configure(state, toplevel, &box);
            }
            break;
        }
        case TOPLEVEL_STATE_FULLSCREEN: {
            if(workspace->fullscreen) {
                return;
            }

            old_workspace->fullscreen = NULL;
            toplevel->workspace = workspace;
            workspace->fullscreen = toplevel;
            toplevel->state = TOPLEVEL_STATE_FULLSCREEN;

            toplevel_configure(state, toplevel, &workspace->output->full_area);
            break;
        }
    }

    workspace_set_active(state, workspace, true);
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

static void
send_size(struct state *state, struct toplevel *toplevel, int width, int height) {
    toplevel->requested_width = width;
    toplevel->requested_height = height;
    toplevel->configure_serial = wlr_xdg_toplevel_set_size(toplevel->wlr_toplevel, width, height);
    transaction_mark_dirty(state, toplevel);
}

void
toplevel_configure(struct state *state, struct toplevel *toplevel, struct wlr_box *box) {
    toplevel->pending = *box;
    toplevel->has_border = should_have_border(state, toplevel);

    if(box->width <= 0 || box->height <= 0) {
        // should choose its own size
        send_size(state, toplevel, 0, 0);
        return;
    }

    // we need to subract the decorations from this toplevel. currently the only type of decoration is the border,
    // but that may change in the future
    int width = box->width;
    int height = box->height;

    if(toplevel->has_border) {
        width -= 2 * state->config.border.width;
        height -= 2 * state->config.border.width;
    }

    // patch this so we dont get negative width/height
    width = MAX(width, 1);
    height = MAX(height, 1);

    if(width == toplevel->requested_width && height == toplevel->requested_height) {
        // this toplevel is ready by default. here we schedule a commit for the workspace, since we dont know if there
        // are going to be other toplevels that are dirty. if not, the idle is going to commit this one or any other
        // that may also not need the commit by the toplevel.
        transaction_schedule_commit(state, toplevel->workspace);
        return;
    };

    send_size(state, toplevel, width, height);
}

static void
set_state(struct toplevel *toplevel, enum toplevel_state new_state) {
    toplevel->state = new_state;
    toplevel->needs_reparenting = true;
}

static void
set_tiled(struct state *state, struct toplevel *toplevel) {
    struct workspace *workspace = toplevel->workspace;

    switch(toplevel->state) {
        case TOPLEVEL_STATE_TILED: {
            return;
        }
        case TOPLEVEL_STATE_FLOAT: {
            wl_list_remove(&toplevel->link);
            break;
        }
        case TOPLEVEL_STATE_FULLSCREEN: {
            workspace->fullscreen = NULL;

            wlr_xdg_toplevel_set_fullscreen(toplevel->wlr_toplevel, false);
            wlr_foreign_toplevel_handle_v1_set_fullscreen(toplevel->foreign_toplevel_handle, false);

            break;
        }
    }

    set_state(toplevel, TOPLEVEL_STATE_TILED);
    layout_add(workspace, toplevel);
    layout_configure(state, workspace);

    set_hacks(toplevel);
}

static void
set_float(struct state *state, struct toplevel *toplevel) {
    struct workspace *workspace = toplevel->workspace;

    switch(toplevel->state) {
        case TOPLEVEL_STATE_TILED: {
            layout_remove(toplevel);
            layout_configure(state, workspace);
            break;
        }
        case TOPLEVEL_STATE_FLOAT: {
            return;
        }
        case TOPLEVEL_STATE_FULLSCREEN: {
            workspace->fullscreen = NULL;

            wlr_xdg_toplevel_set_fullscreen(toplevel->wlr_toplevel, false);
            wlr_foreign_toplevel_handle_v1_set_fullscreen(toplevel->foreign_toplevel_handle, false);
            break;
        }
    }

    wl_list_insert(&workspace->floats, &toplevel->link);
    set_state(toplevel, TOPLEVEL_STATE_FLOAT);

    int width, height;
    default_size(state, toplevel, &width, &height);

    toplevel_configure(state, toplevel,
            &(struct wlr_box){
                    .width = width,
                    .height = height,
            });
    toplevel->needs_centering = true;

    reset_hacks(toplevel);
}

static void
set_fullscreen(struct state *state, struct toplevel *toplevel) {
    struct workspace *workspace = toplevel->workspace;
    if(workspace->fullscreen) {
        return;
    }

    switch(toplevel->state) {
        case TOPLEVEL_STATE_TILED: {
            layout_remove(toplevel);
            layout_configure(state, workspace);
            break;
        }
        case TOPLEVEL_STATE_FLOAT: {
            wl_list_remove(&toplevel->link);
            break;
        }
        case TOPLEVEL_STATE_FULLSCREEN: {
            return;
        }
    }

    // keep note of the previous state in order to restore it later
    toplevel->prev_state = toplevel->state;

    workspace->fullscreen = toplevel;
    set_state(toplevel, TOPLEVEL_STATE_FULLSCREEN);

    reset_hacks(toplevel);

    wlr_xdg_toplevel_set_fullscreen(toplevel->wlr_toplevel, true);
    toplevel_configure(state, toplevel, &workspace->output->full_area);
    wlr_foreign_toplevel_handle_v1_set_fullscreen(toplevel->foreign_toplevel_handle, true);
}

void
toplevel_set_state(struct state *state, struct toplevel *toplevel, enum toplevel_state new_state) {
    if(toplevel == state->grabbed_toplevel) {
        // cant change the state of this toplevel until dropped
        return;
    }

    switch(new_state) {
        case TOPLEVEL_STATE_TILED: {
            set_tiled(state, toplevel);
            break;
        }
        case TOPLEVEL_STATE_FLOAT: {
            set_float(state, toplevel);
            break;
        }
        case TOPLEVEL_STATE_FULLSCREEN: {
            set_fullscreen(state, toplevel);
            break;
        };
    }
}
