#include "transaction.h"

#include "comphy.h"
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
all_ready(struct workspace *workspace) {
    if(workspace->master && workspace->master->is_dirty) {
        wlr_log(WLR_DEBUG, "workspace '%d' master dirty", workspace->idx);
        return false;
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        if(iter->is_dirty) {
            wlr_log(WLR_DEBUG, "workspace '%d' float dirty", workspace->idx);
            return false;
        }
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        if(iter->is_dirty) {
            wlr_log(WLR_DEBUG, "workspace '%d' slave dirty", workspace->idx);
            return false;
        }
    }

    return true;
}

static void
commit(struct state *state, struct toplevel *toplevel) {
    toplevel->current = toplevel->pending;

    if(toplevel->needs_initial_enable) {
        wlr_scene_node_set_enabled(&toplevel->scene_tree->node, true);
        toplevel->needs_initial_enable = false;
    }

    wlr_scene_node_set_position(&toplevel->scene_tree->node, toplevel->current.x, toplevel->current.y);
    wlr_scene_rect_set_size(toplevel->border, toplevel->current.width + 2 * state->config.border.width,
            toplevel->current.height + 2 * state->config.border.width);

    if(toplevel->snapshot_tree) {
        wlr_scene_node_destroy(&toplevel->snapshot_tree->node);
        toplevel->snapshot_tree = NULL;
        // reenable the real buffer
        wlr_scene_node_set_enabled(&toplevel->content_tree->node, true);
    }
}

static void
commit_all(struct state *state, struct workspace *workspace, bool unmark) {
    if(workspace->master) {
        commit(state, workspace->master);
        if(unmark) {
            workspace->master->is_dirty = false;
        }
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        commit(state, iter);
        if(unmark) {
            iter->is_dirty = false;
        }
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        commit(state, iter);
        if(unmark) {
            iter->is_dirty = false;
        }
    }
}

void
transaction_commit(struct state *state, struct toplevel *toplevel) {
    wlr_log(WLR_DEBUG, "toplevel '%p' transaction commit", (void *)toplevel);

    toplevel->is_dirty = false;

    // TODO: handle grabbed

    if(toplevel->state == TOPLEVEL_STATE_FULLSCREEN) {
        // dont need anything else
        commit(state, toplevel);
        return;
    }

    struct workspace *workspace = toplevel->workspace;
    if(!all_ready(workspace)) {
        wlr_log(WLR_DEBUG, "transaction not ready yet");
        // transaction not ready
        return;
    }

    wlr_log(WLR_DEBUG, "transaction ready");

    // all ready, remove the time out and commit all the toplevels
    if(workspace->transaction_time_out) {
        wl_event_source_remove(workspace->transaction_time_out);
        workspace->transaction_time_out = NULL;
    }

    commit_all(state, workspace, false);
}

static void
idle(void *data) {
    struct toplevel *toplevel = data;
    struct state *state = state_get();

    transaction_commit(state, toplevel);
    toplevel->transaction_schedule_idle = NULL;
}

void
transaction_schedule_commit(struct state *state, struct toplevel *toplevel) {
    if(toplevel->transaction_schedule_idle) {
        // already armed
        return;
    }

    struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);
    toplevel->transaction_schedule_idle = wl_event_loop_add_idle(event_loop, idle, toplevel);
}

int
transaction_time_out(void *data) {
    struct workspace *workspace = data;
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "transaction for workspace '%d' timed out", workspace->idx);

    wl_event_source_remove(workspace->transaction_time_out);
    workspace->transaction_time_out = NULL;

    commit_all(state, workspace, true);

    return 0;
}

void
transaction_mark_dirty(struct state *state, struct toplevel *toplevel) {
    toplevel->is_dirty = true;

    // create the snapshot tree to replace it until the new buffer is ready
    wlr_scene_node_set_enabled(&toplevel->content_tree->node, false);
    toplevel->snapshot_tree = scene_tree_snapshot(toplevel->content_tree);

    struct workspace *workspace = toplevel->workspace;
    if(!workspace->transaction_time_out) {
        struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);
        workspace->transaction_time_out = wl_event_loop_add_timer(event_loop, transaction_time_out, workspace);
    }
    wl_event_source_timer_update(workspace->transaction_time_out, COMPHY_TRANSACTION_TIME_OUT_MS);
}
