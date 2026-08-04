#include "operation.h"

#include <wlr/util/log.h>

#include "layout.h"
#include "list_helpers.h"
#include "state.h"
#include "util/macros.h"
#include "util/time_util.h"
#include "workspace.h"

// TODO: if server side then handle the cursor, reference:
// void
// keybind_resize_focused_toplevel(void *data) {
//     struct mwc_toplevel *toplevel = get_pointer_focused_toplevel();
//     if(toplevel == NULL || !toplevel->floating)
//         return;
//
//     uint32_t edges = toplevel_get_closest_corner(server.cursor, toplevel);
//
//     char cursor_image[128] = {0};
//     if(edges & WLR_EDGE_TOP) {
//         strcat(cursor_image, "top_");
//     } else {
//         strcat(cursor_image, "bottom_");
//     }
//     if(edges & WLR_EDGE_LEFT) {
//         strcat(cursor_image, "left_");
//     } else {
//         strcat(cursor_image, "right_");
//     }
//     strcat(cursor_image, "corner");
//
//     wlr_cursor_set_xcursor(server.cursor, server.cursor_mgr, cursor_image);
//
//     server.client_driven_move_resize = false;
//     toplevel_start_resize(toplevel, edges);
// }

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

static void
stop_shared(struct state *state) {
    state->operation = OPERATION_NONE;
    state->grabbed_toplevel = NULL;

    // clear the focus and then give it immediatelly so the client requests a new cursor image, since the server might
    // have been the one who initialized this action and who set its own cursor image
    wlr_seat_pointer_clear_focus(state->seat.wlr_seat);
    cursor_focus(state, time_now_ms(), false);
}

void
operation_stop_move(struct state *state) {
    ASSERT(state->operation == OPERATION_MOVE);

    struct toplevel *toplevel = state->grabbed_toplevel;

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

    stop_shared(state);
}

void
operation_start_resize(struct state *state, struct toplevel *toplevel, u32 edges) {
    if(state->grabbed_toplevel || toplevel->state != TOPLEVEL_STATE_FLOAT) {
        return;
    }

    state->operation = OPERATION_RESIZE;
    state->grabbed_toplevel = toplevel;
    state->grab_x = state->cursor.wlr_cursor->x;
    state->grab_y = state->cursor.wlr_cursor->y;
    state->grabbed_toplevel_initial_box = toplevel->current;
    state->resize_edges = edges;
}

void
operation_stop_resize(struct state *state) {
    struct toplevel *toplevel = state->grabbed_toplevel;
    struct output *largest_output = toplevel_float_largest_output_intersection(state, toplevel);

    if(largest_output != toplevel->workspace->output) {
        toplevel->workspace = largest_output->active_workspace;
        wl_list_remove(&toplevel->link);
        wl_list_insert(&largest_output->active_workspace->floats, &toplevel->link);
    }

    stop_shared(state);
}

void
operation_start_drag(struct state *state) {
    // TODO:
    UNUSED(state);
}

void
operation_stop_drag(struct state *state) {
    // TODO:
    UNUSED(state);
}

void
operation_stop_whatever(struct state *state) {
    switch(state->operation) {
        case OPERATION_NONE: {
            break;
        }
        case OPERATION_MOVE: {
            operation_stop_move(state);
            break;
        }
        case OPERATION_RESIZE: {
            operation_stop_resize(state);
            break;
        }
        case OPERATION_DRAG: {
            operation_stop_drag(state);
            break;
        }
    }
}

static void
move(struct state *state) {
    struct toplevel *toplevel = state->grabbed_toplevel;

    struct wlr_box box = {
            .x = state->grabbed_toplevel_initial_box.x + (state->cursor.wlr_cursor->x - state->grab_x),
            .y = state->grabbed_toplevel_initial_box.y + (state->cursor.wlr_cursor->y - state->grab_y),
            .width = toplevel->current.width,
            .height = toplevel->current.height,
    };

    toplevel_configure(state, toplevel, &box);
}

static void
resize(struct state *state) {
    struct toplevel *toplevel = state->grabbed_toplevel;

    struct wlr_box initial = state->grabbed_toplevel_initial_box;
    struct wlr_box box = state->grabbed_toplevel_initial_box;

    int cursor_x = state->cursor.wlr_cursor->x;
    int cursor_y = state->cursor.wlr_cursor->y;

    int grab_x = state->grab_x;
    int grab_y = state->grab_y;

    int min_width = MAX(toplevel->wlr_toplevel->current.min_width, 10);
    int min_height = MAX(toplevel->wlr_toplevel->current.min_height, 10);

    if(state->resize_edges & WLR_EDGE_TOP) {
        box.y = initial.y + (cursor_y - grab_y);
        box.height = initial.height - (cursor_y - grab_y);
        if(box.height <= min_height) {
            box.y = initial.y + initial.height - min_height;
            box.height = min_height;
        }
    } else if(state->resize_edges & WLR_EDGE_BOTTOM) {
        box.y = initial.y;
        box.height = initial.height + (cursor_y - grab_y);
        if(box.height <= min_height) {
            box.height = min_height;
        }
    }

    if(state->resize_edges & WLR_EDGE_LEFT) {
        box.x = initial.x + (cursor_x - grab_x);
        box.width = initial.width - (cursor_x - grab_x);
        if(box.width <= min_width) {
            box.x = initial.x + initial.width - min_width;
            box.width = min_width;
        }
    } else if(state->resize_edges & WLR_EDGE_RIGHT) {
        box.x = initial.x;
        box.width = initial.width + (cursor_x - grab_x);
        if(box.width <= min_width) {
            box.width = min_width;
        }
    }

    toplevel_configure(state, toplevel, &box);
}

void
operation_tick(struct state *state) {
    switch(state->operation) {
        case OPERATION_NONE: {
            break;
        }
        case OPERATION_MOVE: {
            move(state);
        }
        case OPERATION_RESIZE: {
            resize(state);
        }
        case OPERATION_DRAG: {
            // TODO: implement
            // drag(state);
        } break;
    }
}
