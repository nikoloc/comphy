#include "transaction.h"

#include "comphy.h"
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
        return false;
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        if(iter->is_dirty) {
            return false;
        }
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        if(iter->is_dirty) {
            return false;
        }
    }

    return true;
}

static void
commit(struct toplevel *toplevel) {
    toplevel->current = toplevel->pending;

    // reenable the real buffer
    wlr_scene_node_destroy(&toplevel->snapshot_tree->node);
    toplevel->snapshot_tree = NULL;

    wlr_scene_node_set_position(&toplevel->scene_tree->node, toplevel->current.x, toplevel->current.y);

    wlr_scene_node_set_enabled(&toplevel->content_tree->node, true);
    // TODO: fix harcoded
    wlr_scene_rect_set_size(toplevel->border, toplevel->current.width + 6, toplevel->current.height + 6);
}

static void
commit_all(struct workspace *workspace) {
    if(workspace->master) {
        commit(workspace->master);
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        commit(iter);
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        commit(iter);
    }
}

void
transaction_commit(struct toplevel *toplevel) {
    toplevel->is_dirty = false;

    // TODO: handle grabbed

    if(toplevel->state == TOPLEVEL_STATE_FULLSCREEN) {
        // dont need anything else
        commit(toplevel);
        return;
    }

    struct workspace *workspace = toplevel->workspace;
    if(!all_ready(workspace)) {
        // transaction not ready
        return;
    }

    commit_all(workspace);
}

static void
idle(void *data) {
    struct toplevel *toplevel = data;

    transaction_commit(toplevel);
    toplevel->transaction_schedule_idle = NULL;
}

void
transaction_schedule_commit(struct state *state, struct toplevel *toplevel) {
    struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);

    toplevel->transaction_schedule_idle = wl_event_loop_add_idle(event_loop, idle, toplevel);
}

int
transaction_time_out(void *data) {
    struct toplevel *toplevel = data;

    transaction_commit(toplevel);

    wl_event_source_remove(toplevel->transaction_time_out);
    toplevel->transaction_time_out = NULL;
    return 0;
}

void
transaction_mark_dirty(struct state *state, struct toplevel *toplevel) {
    toplevel->is_dirty = true;

    // create the snapshot tree to replace it until the new buffer is ready
    wlr_scene_node_set_enabled(&toplevel->content_tree->node, false);
    toplevel->snapshot_tree = scene_tree_snapshot(toplevel->content_tree);
    // TODO: fix hardcoded
    wlr_scene_node_set_position(&toplevel->snapshot_tree->node, 3, 3);

    struct wl_event_loop *event_loop = wl_display_get_event_loop(state->display);
    toplevel->transaction_time_out = wl_event_loop_add_timer(event_loop, transaction_time_out, toplevel);
    wl_event_source_timer_update(toplevel->transaction_time_out, COMPHY_TRANSACTION_TIME_OUT_MS);
}
