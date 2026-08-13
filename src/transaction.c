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

static bool
all_ready(struct state *state, struct workspace *workspace) {
    struct wlr_box dummy;
    if(state->operation == OPERATION_MOVE && state->grabbed_toplevel &&
            wlr_box_intersection(&dummy, &state->grabbed_toplevel->pending, &workspace->output->full_area) &&
            state->grabbed_toplevel->transaction_state == TRANSACTION_STATE_DIRTY) {
        return false;
    }

    if(workspace->master && workspace->master->transaction_state == TRANSACTION_STATE_DIRTY) {
        wlr_log(WLR_DEBUG, "workspace '%d' master dirty", workspace->idx);
        return false;
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        if(iter->transaction_state == TRANSACTION_STATE_DIRTY) {
            wlr_log(WLR_DEBUG, "workspace '%d' float dirty", workspace->idx);
            return false;
        }
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        if(iter->transaction_state == TRANSACTION_STATE_DIRTY) {
            wlr_log(WLR_DEBUG, "workspace '%d' slave dirty", workspace->idx);
            return false;
        }
    }

    return true;
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
commit(struct state *state, struct toplevel *toplevel) {
    if(toplevel->transaction_state == TRANSACTION_STATE_DIRTY) {
        // request a new frame since we are commiting it with no good state
        toplevel_send_frame_done(toplevel);
    }

    // return the transaction state for this toplevel to the default one
    toplevel->transaction_state = TRANSACTION_STATE_CLEAN;

    // update the presentation
    if(toplevel->needs_initial_enable) {
        wlr_scene_node_set_enabled(&toplevel->scene_tree->node, true);
        toplevel->needs_initial_enable = false;
    }

    if(toplevel->state == TOPLEVEL_STATE_FLOAT && toplevel->needs_centering) {
        // this means the client timed out and its pending stuff is all zeros, we patch it with current
        struct wlr_box output_box = toplevel->workspace->output->usable_area;
        toplevel->current = wlr_box_centered_in(&output_box, toplevel->current.width, toplevel->current.height);

        toplevel->needs_centering = false;
    } else {
        toplevel->current = toplevel->pending;
    }

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
}

static void
remove_ghosts(struct workspace *workspace) {
    struct toplevel *iter, *tmp;
    wl_list_for_each_safe(iter, tmp, &workspace->ghosts, link) {
        toplevel_finalize_destroy(iter);
    }

    // reset the list
    wl_list_init(&workspace->ghosts);
}

static void
commit_all(struct state *state, struct workspace *workspace) {
    struct wlr_box dummy;
    if(state->operation == OPERATION_MOVE &&
            wlr_box_intersection(&dummy, &state->grabbed_toplevel->pending, &workspace->output->full_area)) {
        commit(state, state->grabbed_toplevel);
    }

    if(workspace->master) {
        commit(state, workspace->master);
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        commit(state, iter);
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        commit(state, iter);
    }

    remove_ghosts(workspace);
    workspace->has_dirty = false;
}

static void
remove_time_out(struct workspace *workspace) {
    if(workspace->transaction_time_out) {
        wl_event_source_remove(workspace->transaction_time_out);
        workspace->transaction_time_out = NULL;
    }
}

void
transaction_commit(struct state *state, struct toplevel *toplevel) {
    if(toplevel->transaction_state != TRANSACTION_STATE_DIRTY) {
        ASSERT(toplevel->state == TOPLEVEL_STATE_FLOAT);

        // not needed, but means the toplevel changed its size on its own. allow it
        commit(state, toplevel);
        return;
    }

    wlr_log(WLR_DEBUG, "toplevel '%p' transaction commit", (void *)toplevel);

    toplevel->transaction_state = TRANSACTION_STATE_READY;

    struct workspace *workspace = toplevel->workspace;
    if(toplevel->state == TOPLEVEL_STATE_FULLSCREEN) {
        // dont need anything else
        commit(state, toplevel);
        remove_time_out(workspace);
        return;
    }

    if(!all_ready(state, workspace)) {
        wlr_log(WLR_DEBUG, "transaction not ready yet");
        // transaction not ready
        return;
    }

    wlr_log(WLR_DEBUG, "transaction ready");
    // all ready, remove the time out and commit all the toplevels
    remove_time_out(workspace);
    commit_all(state, workspace);
}

int
transaction_time_out(void *data) {
    struct workspace *workspace = data;
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "transaction for workspace '%d' timed out", workspace->idx);

    remove_time_out(workspace);
    commit_all(state, workspace);

    return 0;
}

static void
idle(void *data) {
    struct workspace *workspace = data;
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "workspace '%d' commited on idle", workspace->idx);

    commit_all(state, workspace);
    workspace->transaction_schedule = NULL;
}

void
transaction_schedule_commit(struct state *state, struct workspace *workspace) {
    if(workspace->transaction_schedule || workspace->has_dirty) {
        // already armed or there is a dirty toplevel here already, so we dont want an idle
        return;
    }

    struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);
    workspace->transaction_schedule = wl_event_loop_add_idle(event_loop, idle, workspace);
}

void
transaction_mark_dirty(struct state *state, struct toplevel *toplevel) {
    if(toplevel->transaction_state == TRANSACTION_STATE_DIRTY) {
        // already marked
        return;
    }

    toplevel->transaction_state = TRANSACTION_STATE_DIRTY;

    if(toplevel->snapshot_tree) {
        // leftover if the previous transaction did not finish
        wlr_scene_node_destroy(&toplevel->snapshot_tree->node);
    }

    // create the snapshot tree to replace it until the new buffer is ready
    wlr_scene_node_set_enabled(&toplevel->content_tree->node, false);
    toplevel->snapshot_tree = scene_tree_snapshot(toplevel->content_tree);

    struct workspace *workspace = toplevel->workspace;
    workspace->has_dirty = true;
    if(!workspace->transaction_time_out) {
        // if this is the first toplevel for the transaction create the timer
        struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);
        workspace->transaction_time_out = wl_event_loop_add_timer(event_loop, transaction_time_out, workspace);
    }
    wl_event_source_timer_update(workspace->transaction_time_out, COMPHY_TRANSACTION_TIME_OUT_MS);

    if(workspace->transaction_schedule) {
        // remove an idle if there was one armed
        wl_event_source_remove(workspace->transaction_schedule);
        workspace->transaction_schedule = NULL;
        return;
    }
}
