#include "operation.h"

#include "layout.h"
#include "list_helpers.h"
#include "state.h"
#include "util/macros.h"
#include "wlr/util/log.h"
#include "workspace.h"

void
operation_start_move(struct state *state, struct toplevel *toplevel) {
    if(state->operation) {
        // if there is some operation already skip it
        return;
    }

    state->operation = OPERATION_MOVE;
    state->grabbed_toplevel = toplevel;

    state->grab_x = state->cursor.wlr_cursor->x;
    state->grab_y = state->cursor.wlr_cursor->y;

    // TODO: should we handle the transaction here
    state->grabbed_toplevel_initial_box = toplevel->current;

    // remove the toplevel and fix the layout
    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        wl_list_remove(&toplevel->link);
    } else {
        struct workspace *workspace = toplevel->workspace;
        if(toplevel == workspace->master) {
            struct wl_list *next_master = wl_list_last(&toplevel->workspace->slaves);
            if(next_master) {
                workspace->master = CONTAINER_OF(next_master, struct toplevel, link);
                wl_list_remove(next_master);
            } else {
                workspace->master = NULL;
            }
        } else {
            wl_list_remove(&toplevel->link);
        }

        // fix the layout by inserting new master
        layout_configure(state, toplevel->workspace);
    }

    // move this toplevel to the grab tree
    wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.grab);
}

static void
insert_layout_at_cursor(struct state *state, struct toplevel *toplevel) {
    int x = state->cursor.wlr_cursor->x;
    int y = state->cursor.wlr_cursor->y;

    struct output *output = cursor_get_output(state);
    if(!output) {
        // if for any reason this failed back to the first output
        struct wl_list *first = wl_list_first(&state->outputs);
        if(!first) {
            // in the middle of the resize the user disconnected the output
            wlr_log(WLR_ERROR, "you are a demon for doing this tbh");
            exit(1);
        }

        output = CONTAINER_OF(first, struct output, link);
        // TODO: insert it last in this case
        return;
    }

    // relative coords on this output
    int dx = output->full_area.x - x;
    int dy = output->full_area.y - y;

    // if over the master insert it as master, else insert it as slave. for the inserting take the approach of inserting
    // before the slave if the cursor is over the top part of the toplevel and vice versa.
    if(dx < output->full_area.width * state->config.master_ratio) {
        // TODO: finish this when the layout is done
    } else {
        // TODO: finish this when the layout is done
    }
}

void
operation_stop_move(struct state *state) {
    ASSERT(state->operation == OPERATION_MOVE);

    state->operation = OPERATION_NONE;

    struct toplevel *toplevel = state->grabbed_toplevel;
    state->grabbed_toplevel = NULL;

    // reinsert the toplevel. NOTE: for floating we take the largest intersection output. for tiled we insert it into
    // the layout at the cursor position
    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        struct output *output = toplevel_float_largest_output_intersection(state, toplevel);
        if(!output) {
            // if for any reason this failed back to the first output
            struct wl_list *first = wl_list_first(&state->outputs);
            if(!first) {
                // in the middle of the resize the user disconnected the output
                wlr_log(WLR_ERROR, "you are a demon for doing this tbh");
                exit(1);
            }

            output = CONTAINER_OF(first, struct output, link);
        }

        wl_list_insert(&output->active_workspace->floats, &toplevel->link);
        toplevel->workspace = output->active_workspace;

        // return it to the float tree
        wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.floats);
    } else {
        insert_layout_at_cursor(state, toplevel);

        // return it to the tiled tree
        wlr_scene_node_reparent(&toplevel->scene_tree->node, state->scene.trees.tiled);
    }
}

void
operation_start_resize(struct state *state, struct toplevel *toplevel, u32 edges);

void
operation_stop_resize(struct state *state);

// TODO: add whatever we need here
void
operation_start_drag(struct state *state);

void
operation_stop_drag(struct state *state);

void
operation_stop_whatever(struct state *state);
