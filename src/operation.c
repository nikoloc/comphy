#include "operation.h"

#include <wlr/types/wlr_data_device.h>
#include <wlr/util/log.h>
#include <wlr/xcursor.h>

#include "box_helpers.h"
#include "layout.h"
#include "list_helpers.h"
#include "state.h"
#include "util/macros.h"
#include "util/time_util.h"
#include "workspace.h"

void
operation_start_move(struct state *state, struct toplevel *toplevel, bool server_inited) {
    if(state->operation) {
        // if there is some operation already skip it
        return;
    }

    state->operation = OPERATION_MOVE;
    state->grabbed_toplevel = toplevel;

    state->grab_x = state->cursor.wlr_cursor->x;
    state->grab_y = state->cursor.wlr_cursor->y;

    state->grabbed_toplevel_initial_box = toplevel->current;

    // remove the toplevel and fix the layout
    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        // raise before removing!
        toplevel_raise(toplevel);
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
    toplevel->needs_reparenting = true;

    state->operation_server_inited = server_inited;
    if(server_inited) {
        // set the cursor image
        cursor_set_image(state, "grab");
    }
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
    }

    struct workspace *workspace = output->active_workspace;

    if(!workspace->master) {
        workspace->master = toplevel;
        layout_configure(state, workspace);
        return;
    }

    // doing all the calcalations for the layout just seems bothersome; instead loop throught all see where you are
    struct wlr_box box = workspace->master->current;
    wlr_box_add_gaps(&box, state->config.gaps.inner);

    if(wlr_box_contains_point(&box, x, y)) {
        // we are on top of master, check left or right
        if(x < box.x + box.width / 2) {
            // left, insert as mastter
            wl_list_insert(&workspace->slaves, &workspace->master->link);
            workspace->master = toplevel;
        } else {
            // as slave
            wl_list_insert(workspace->slaves.prev, &toplevel->link);
        }

        layout_configure(state, workspace);
        return;
    }

    // go through slaves and check
    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->slaves, link) {
        box = iter->current;
        wlr_box_add_gaps(&box, state->config.gaps.inner);

        if(wlr_box_contains_point(&box, x, y)) {
            if(y < box.y + box.height / 2) {
                // above, insert before this one
                wl_list_insert(iter->link.prev, &toplevel->link);
            } else {
                // after
                wl_list_insert(&iter->link, &toplevel->link);
            }

            layout_configure(state, workspace);
            return;
        }
    }

    // if nothing (e.g. over a panel or outer gaps) insert regular
    layout_add(workspace, toplevel);
    layout_configure(state, workspace);
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
    } else {
        insert_layout_at_cursor(state, toplevel);
    }

    // return it to the right tree
    toplevel->needs_reparenting = true;
    stop_shared(state);
}

void
operation_start_resize(struct state *state, struct toplevel *toplevel, u32 edges, bool server_inited) {
    if(state->grabbed_toplevel || toplevel->state != TOPLEVEL_STATE_FLOAT) {
        return;
    }

    state->operation = OPERATION_RESIZE;
    state->grabbed_toplevel = toplevel;
    state->grab_x = state->cursor.wlr_cursor->x;
    state->grab_y = state->cursor.wlr_cursor->y;
    state->grabbed_toplevel_initial_box = toplevel->current;
    state->resize_edges = edges;

    state->operation_server_inited = server_inited;
    if(server_inited) {
        // set the cursor image
        cursor_set_image(state, (char *)wlr_xcursor_get_resize_name(edges));
    }

    if(toplevel->state == TOPLEVEL_STATE_FLOAT) {
        toplevel_raise(toplevel);
    }
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

    toplevel->needs_reparenting = true;
    stop_shared(state);
}

static void
handle_destroy_drag(struct wl_listener *listener, void *data) {
    UNUSED(listener), UNUSED(data);

    struct state *state = state_get();

    state->operation = OPERATION_NONE;
    wl_list_remove(&state->seat.destroy_drag.link);

    cursor_focus(state, time_now_ms(), false);
}

void
operation_start_drag(struct state *state, struct wlr_drag *drag) {
    state->operation = OPERATION_DRAG;

    if(drag->icon) {
        wlr_scene_drag_icon_create(state->scene.trees.grab, drag->icon);
    }

    cursor_set_image(state, "grab");

    state->seat.destroy_drag.notify = handle_destroy_drag;
    wl_signal_add(&drag->events.destroy, &state->seat.destroy_drag);
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

    if(toplevel->has_border) {
        min_width += 2 * state->config.border.width;
        min_height += 2 * state->config.border.width;
    }

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
            break;
        }
        case OPERATION_RESIZE: {
            resize(state);
            break;
        }
        case OPERATION_DRAG: {
            wlr_scene_node_set_position(&state->scene.trees.grab->node, state->cursor.wlr_cursor->x,
                    state->cursor.wlr_cursor->y);
            break;
        }
    }
}
