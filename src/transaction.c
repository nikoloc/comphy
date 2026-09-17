#include "transaction.h"

#include "box_helpers.h"
#include "comphy.h"
#include "util/macros.h"
#include "wlr/util/log.h"
#include "workspace.h"

// thanks to `fx_comp` and `sway` for the implementation
static void
scene_node_snapshot(struct wlr_scene_node *node, int lx, int ly, struct wlr_scene_tree *snapshot_tree) {
    lx += node->x;
    ly += node->y;

    struct wlr_scene_node *snapshot_node = NULL;
    switch(node->type) {
        case WLR_SCENE_NODE_TREE: {
            struct wlr_scene_tree *scene_tree = wlr_scene_tree_from_node(node);

            struct wlr_scene_node *child;
            wl_list_for_each(child, &scene_tree->children, link) {
                scene_node_snapshot(child, lx, ly, snapshot_tree);
            }
            break;
        }
        case WLR_SCENE_NODE_BUFFER: {
            struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);

            struct wlr_scene_buffer *snapshot_buffer = wlr_scene_buffer_create(snapshot_tree, NULL);
            snapshot_node = &snapshot_buffer->node;
            snapshot_buffer->node.data = scene_buffer->node.data;

            wlr_scene_buffer_set_dest_size(snapshot_buffer, scene_buffer->dst_width, scene_buffer->dst_height);
            wlr_scene_buffer_set_opaque_region(snapshot_buffer, &scene_buffer->opaque_region);
            wlr_scene_buffer_set_source_box(snapshot_buffer, &scene_buffer->src_box);
            wlr_scene_buffer_set_transform(snapshot_buffer, scene_buffer->transform);
            wlr_scene_buffer_set_filter_mode(snapshot_buffer, scene_buffer->filter_mode);
            wlr_scene_buffer_set_opacity(snapshot_buffer, scene_buffer->opacity);

            snapshot_buffer->node.data = scene_buffer->node.data;

            struct wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);
            if(scene_surface && scene_surface->surface->buffer) {
                wlr_scene_buffer_set_buffer(snapshot_buffer, &scene_surface->surface->buffer->base);
            } else {
                wlr_scene_buffer_set_buffer(snapshot_buffer, scene_buffer->buffer);
            }
            break;
        }
        case WLR_SCENE_NODE_RECT: {
            break;
        }
    }

    if(snapshot_node) {
        wlr_scene_node_set_position(snapshot_node, lx, ly);
    }
}

static struct wlr_scene_tree *
scene_tree_snapshot(struct wlr_scene_tree *tree) {
    struct wlr_scene_tree *parent = tree->node.parent;
    struct wlr_scene_tree *snapshot = wlr_scene_tree_create(parent);

    // disable and enable the snapshot tree like so to atomically update the scene-graph. this will prevent
    // over-damaging or other weirdness.
    wlr_scene_node_set_enabled(&snapshot->node, false);
    scene_node_snapshot(&tree->node, 0, 0, snapshot);
    wlr_scene_node_set_enabled(&snapshot->node, true);

    return snapshot;
}

static void
reparent(struct state *state, struct toplevel *toplevel) {
    if(toplevel == state->grabbed_toplevel) {
        wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.grab);
    } else {
        switch(toplevel->state) {
            case TOPLEVEL_STATE_TILED: {
                wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.tiled);
                break;
            }
            case TOPLEVEL_STATE_FLOAT: {
                wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.floats);
                toplevel_handle_parents_and_children(toplevel);
                break;
            }
            case TOPLEVEL_STATE_FULLSCREEN: {
                wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.fullscreen);
                break;
            }
        }
    }

    toplevel->needs_reparenting = false;
}

static void
clip(struct state *state, struct toplevel *toplevel) {
    struct wlr_box *geometry = &toplevel->wlr_toplevel->base->geometry;
    int width = toplevel->current.width;
    int height = toplevel->current.height;

    if(toplevel->has_border) {
        width -= 2 * state->config.border.width;
        height -= 2 * state->config.border.width;
    }

    // NOTE: we start from geometry.x and geometry.y in order to skip the shadow, effects etc
    struct wlr_box clip = (struct wlr_box){
            .x = geometry->x,
            .y = geometry->y,
            .width = width,
            .height = height,
    };

    wlr_scene_subsurface_tree_set_clip(&toplevel->content_tree->node, &clip);

    // remove the clip from popups
    struct wlr_scene_node *iter;
    wl_list_for_each(iter, &toplevel->scene_tree->children, link) {
        enum view *view = iter->data;
        if(view && *view == VIEW_POPUP) {
            wlr_scene_subsurface_tree_set_clip(iter, NULL);
        }
    }
}

static void
center_float(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

    if(toplevel->pending.width == 0 || toplevel->pending.height) {
        // CONTINUE HERE
    }

    struct wlr_box output_box = toplevel->workspace->output->usable_area;
    toplevel->pending.x = output_box.x + (output_box.width - toplevel->pending.width) / 2;
    toplevel->pending.y = output_box.y + (output_box.height - toplevel->pending.height) / 2;

    toplevel->needs_centering = false;
}

static void
conform_to_client_geometry(struct state *state, struct toplevel *toplevel) {
    struct wlr_box *geometry = &toplevel->wlr_toplevel->base->geometry;

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
}

static void
commit(struct state *state, struct toplevel *toplevel) {
    if(toplevel->transaction_state == TRANSACTION_STATE_DIRTY) {
        // request a new frame since we are commiting it with no good state
        toplevel_send_frame_done(toplevel);
    }

    // reset the transaction state
    toplevel->transaction_state = TRANSACTION_STATE_CLEAN;

    // patch own size for floating clients
    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        conform_to_client_geometry(state, toplevel);
    }

    // commit the new state
    toplevel->current = toplevel->pending;

    // update the presentation
    clip(state, toplevel);
    if(toplevel->needs_reparenting) {
        reparent(state, toplevel);
    }

    wlr_scene_node_set_position(&toplevel->scene_tree->node, toplevel->current.x, toplevel->current.y);
    if(toplevel->has_border) {
        wlr_scene_node_set_position(&toplevel->content_tree->node, state->config.border.width,
                state->config.border.width);
    } else {
        wlr_scene_node_set_position(&toplevel->content_tree->node, 0, 0);
    }

    wlr_scene_rect_set_size(toplevel->border, toplevel->current.width, toplevel->current.height);
    wlr_scene_node_set_enabled(&toplevel->border->node, toplevel->has_border);

    if(toplevel->snapshot_tree) {
        wlr_scene_node_destroy(&toplevel->snapshot_tree->node);
        toplevel->snapshot_tree = NULL;
        // reenable the real buffer
        wlr_scene_node_set_enabled(&toplevel->content_tree->node, true);
    }

    if(toplevel->needs_initial_enable) {
        wlr_scene_node_set_enabled(&toplevel->scene_tree->node, true);
        toplevel->needs_initial_enable = false;
    }
}

static void
remove_time_out(struct state *state) {
    if(state->transaction.time_out) {
        wl_event_source_remove(state->transaction.time_out);
        state->transaction.time_out = NULL;
    }
}

static void
remove_ghosts(struct state *state) {
    struct toplevel *iter, *tmp;
    wl_list_for_each_safe(iter, tmp, &state->transaction.ghosts, link) {
        if(iter->is_destroyed) {
            toplevel_finalize_destroy(iter);
        } else {
            // else we wait for the destroy callback and unmark it as ghost in order for everything to cleanup
            iter->is_ghost = false;
        }
    }

    // reset the list
    wl_list_init(&state->transaction.ghosts);
}

static void
show_workspace(struct state *state) {
    struct workspace *workspace = state->active_workspace;
    struct workspace *presented = state->active_workspace->output->presented_workspace;

    state->active_workspace->output->presented_workspace = workspace;

    if(presented && presented != workspace && presented->output == workspace->output) {
        workspace_show_toplevels(presented, false);
    }

    workspace_show_toplevels(workspace, true);
}

static void
perform_transaction(struct state *state) {
    for(size_t i = 0; i < state->transaction.toplevels.len; i++) {
        struct toplevel *iter = state->transaction.toplevels.data[i];

        if(iter->is_destroyed) {
            toplevel_finalize_destroy(iter);
        } else {
            commit(state, iter);
        }
    }

    // reset the list and count
    state->transaction.toplevels.len = 0;
    state->transaction.dirty_count = 0;

    remove_ghosts(state);
    show_workspace(state);
    remove_time_out(state);
}

void
transaction_commit(struct state *state, struct toplevel *toplevel) {
    ASSERT(toplevel->transaction_state == TRANSACTION_STATE_DIRTY);

    wlr_log(WLR_DEBUG, "toplevel '%p' transaction commit", (void *)toplevel);

    toplevel->transaction_state = TRANSACTION_STATE_READY;
    state->transaction.dirty_count--;

    if(state->transaction.dirty_count > 0) {
        wlr_log(WLR_DEBUG, "transaction not ready yet, %d left", state->transaction.dirty_count);
        return;
    }

    wlr_log(WLR_DEBUG, "transaction ready");
    perform_transaction(state);
}

static int
time_out(void *data) {
    UNUSED(data);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "transaction timed out");
    perform_transaction(state);

    return 0;
}

static void
idle(void *data) {
    UNUSED(data);

    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "transaction commited on idle");
    perform_transaction(state);

    state->transaction.schedule = NULL;
}

static void
schedule_stuff(struct state *state) {
    struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);

    if(state->transaction.schedule) {
        // already armed so two possibilities
        //      1) there is still no dirty toplevels, so we need this schedule to be there still or
        //      2) dirty toplevel has been added so we need to remove it, and instead wait for it, or the timeout
        if(state->transaction.dirty_count > 0) {
            wl_event_source_remove(state->transaction.schedule);
            state->transaction.schedule = NULL;
        }
    } else if(state->transaction.dirty_count == 0) {
        state->transaction.schedule = wl_event_loop_add_idle(event_loop, idle, NULL);
    }

    if(!state->transaction.time_out) {
        // if this is the first toplevel for the transaction create the timer
        struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);
        state->transaction.time_out = wl_event_loop_add_timer(event_loop, time_out, NULL);
    }

    wl_event_source_timer_update(state->transaction.time_out, COMPHY_TRANSACTION_TIME_OUT_MS);
}

static void
send_size(struct toplevel *toplevel, int width, int height) {
    toplevel->requested_width = width;
    toplevel->requested_height = height;

    toplevel->configure_serial = wlr_xdg_toplevel_set_size(toplevel->wlr_toplevel, width, height);
}

static void
remove_decorations(struct state *state, struct toplevel *toplevel, int *width, int *height) {
    // we need to subract the decorations from this toplevel. currently the only type of decoration is the border,
    // but that may change in the future
    *width = toplevel->pending.width;
    *height = toplevel->pending.height;

    if(toplevel->has_border) {
        *width -= 2 * state->config.border.width;
        *height -= 2 * state->config.border.width;
    }

    // patch this so we dont get negative width/height
    *width = MAX(*width, 1);
    *height = MAX(*height, 1);
}

static bool
should_have_border(struct state *state, struct toplevel *toplevel) {
    bool smart_gaps = state->config.gaps.smart && toplevel == toplevel->workspace->master &&
                      wl_list_empty(&toplevel->workspace->slaves);

    return toplevel->state != TOPLEVEL_STATE_FULLSCREEN && !smart_gaps;
}

static bool
configure(struct state *state, struct toplevel *toplevel, struct wlr_box *box) {
    toplevel->pending = *box;
    toplevel->has_border = should_have_border(state, toplevel);

    if(box->width <= 0 || box->height <= 0) {
        // should choose its own size
        send_size(toplevel, 0, 0);
        return true;
    }

    int width, height;
    remove_decorations(state, toplevel, &width, &height);

    bool is_dirty = width != toplevel->requested_width || height != toplevel->requested_height;
    send_size(toplevel, width, height);

    return is_dirty;
}

void
transaction_add_dirty(struct state *state, struct toplevel *toplevel, struct wlr_box *box) {
    bool is_dirty = configure(state, toplevel, box);
    if(toplevel->transaction_state == TRANSACTION_STATE_DIRTY) {
        // already marked
        return;
    }

    // this is the transaction logic: we add all the toplevels send to the list, but only mark it as dirty if
    // their size changed. for such toplevels we dont need to wait for the commit.
    if(is_dirty) {
        toplevel->transaction_state = TRANSACTION_STATE_DIRTY;
        state->transaction.dirty_count++;

        wlr_log(WLR_DEBUG, "toplevel '%p' marked dirty, dirty count is %d", (void *)toplevel,
                state->transaction.dirty_count);
    } else {
        // mark this toplevel as ready, this is needed in order to differentiate toplevels that are clean, but a part of
        // transaction and the ones that are clean and not part of the transaction
        toplevel->transaction_state = TRANSACTION_STATE_READY;
    }

    toplevel_ptr_array_push(&state->transaction.toplevels, toplevel);

    if(toplevel->snapshot_tree) {
        // leftover if the previous transaction did not finish
        wlr_scene_node_destroy(&toplevel->snapshot_tree->node);
    }
    // create the snapshot tree to replace it until the new buffer is ready
    wlr_scene_node_set_enabled(&toplevel->content_tree->node, false);
    toplevel->snapshot_tree = scene_tree_snapshot(toplevel->content_tree);
    // if the toplevel is not on screen currently the scene api does not send the frame events. however, we dont
    // want to wait for the toplevel to become visible in order to get its new state applyed. hence, we send a
    // single frame done event in order to force it to commit to the new size.
    toplevel_send_frame_done(toplevel);

    schedule_stuff(state);
}

static int
find_in_toplevels(struct state *state, struct toplevel *toplevel) {
    for(size_t i = 0; i < state->transaction.toplevels.len; i++) {
        struct toplevel *iter = state->transaction.toplevels.data[i];

        if(iter == toplevel) {
            return i;
        }
    }

    return -1;
}

void
transaction_add_auto(struct state *state, struct toplevel *toplevel) {
    // this is called whenever we want to conform to the clients chosen size
    // we set its pending state to the reported geometry
    if(toplevel->transaction_state == TRANSACTION_STATE_DIRTY) {
        // already marked as dirty, so that wins since the client is going to commit the new size anyway
        return;
    }

    if(toplevel->transaction_state == TRANSACTION_STATE_CLEAN) {
        // add to the list if not
        toplevel_ptr_array_push(&state->transaction.toplevels, toplevel);
    }

    conform_to_client_geometry(state, toplevel);
    toplevel->transaction_state = TRANSACTION_STATE_READY;
    schedule_stuff(state);
}

static void
remove_toplevel(struct state *state, struct toplevel *toplevel) {
    if(toplevel->transaction_state == TRANSACTION_STATE_CLEAN) {
        return;
    }

    // this toplevel is part of the current transaction, meaning its somewhere in the list
    // `state->transaction.toplevels`. we find it and remove it from the list
    int idx = find_in_toplevels(state, toplevel);
    ASSERT(idx >= 0);
    toplevel_ptr_array_remove_fast(&state->transaction.toplevels, idx);

    if(toplevel->transaction_state == TRANSACTION_STATE_DIRTY) {
        // also decrese the count
        ASSERT(state->transaction.dirty_count > 0);
        state->transaction.dirty_count--;
    }

    toplevel->transaction_state = TRANSACTION_STATE_CLEAN;
}

void
transaction_add_ghost(struct state *state, struct toplevel *toplevel) {
    toplevel->is_ghost = true;
    wl_list_insert(&state->transaction.ghosts, &toplevel->link);
    // remove if in the transaction currently
    remove_toplevel(state, toplevel);
    schedule_stuff(state);
}

void
transaction_schedule(struct state *state) {
    schedule_stuff(state);
}

void
transaction_add_ready(struct state *state, struct toplevel *toplevel) {
    // this is called whenever there is a change not size related that needs a transaction
    if(toplevel->transaction_state) {
        // already part of the transaction
        return;
    }

    toplevel_ptr_array_push(&state->transaction.toplevels, toplevel);
    toplevel->transaction_state = TRANSACTION_STATE_READY;
    schedule_stuff(state);
}
