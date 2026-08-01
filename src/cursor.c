#include "cursor.h"

#include <wlr/util/log.h>

#include "comphy.h"
#include "keyboard.h"
#include "output.h"
#include "state.h"
#include "util/macros.h"
#include "util/time_util.h"
#include "view.h"

void
cursor_set_image(struct state *state, char *image) {
    wlr_cursor_set_xcursor(state->cursor.wlr_cursor, state->cursor.xcursor_mgr, image);
}

void
cursor_focus(struct state *state, u32 time_ms, bool handle_keyboard_focus) {
    double sx, sy;
    struct wlr_surface *surface = NULL;
    enum view *view = view_at(state, state->cursor.wlr_cursor->x, state->cursor.wlr_cursor->y, &surface, &sx, &sy);

    if(!view) {
        cursor_set_image(state, "default");
        wlr_seat_pointer_clear_focus(state->seat.wlr_seat);
        // we are done here
        return;
    }

    if(handle_keyboard_focus) {
        view_focus(state, view);
    }

    // TODO: contraints
    // struct wlr_pointer_constraint_v1 *wlr_constraint =
    //         wlr_pointer_constraints_v1_constraint_for_surface(server.pointer_contrains_manager, surface,
    //         server.seat);
    // if(wlr_constraint == NULL || wlr_constraint->data == NULL) {
    //     server.current_constraint = NULL;
    // } else {
    //     constraint_set_as_current(wlr_constraint->data);
    // }

    wlr_seat_pointer_notify_enter(state->seat.wlr_seat, surface, sx, sy);
    wlr_seat_pointer_notify_motion(state->seat.wlr_seat, time_ms, sx, sy);
}

static void
handle_motion_shared(struct state *state, u32 time) {
    // get the output that the cursor is on currently
    struct wlr_output *wlr_output =
            wlr_output_layout_output_at(state->output_layout, state->cursor.wlr_cursor->x, state->cursor.wlr_cursor->y);
    struct output *output = wlr_output->data;

    // switch the active workspace if cross monitor
    state->active_workspace = output->active_workspace;

    if(state->operation) {
        // perform the next tick of operation, that is move/resize grabbed toplevel or drag
        operation_tick(state);
    }

    // finally, handle pointer focus
    cursor_focus(state, time, true);
}

static void
handle_motion(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_pointer_motion_event *event = data;
    struct state *state = state_get();

    // TODO: when contraints and relative pointer
    // constrain_apply_to_move(&event->delta_x, &event->delta_y);

    // wlr_relative_pointer_manager_v1_send_relative_motion(server.relative_pointer_manager,
    //                                                    server.seat,
    //                                                    (uint64_t)event->time_msec * 1000,
    //                                                    event->delta_x, event->delta_y,
    //                                                    event->unaccel_dx, event->unaccel_dy);

    wlr_cursor_move(state->cursor.wlr_cursor, &event->pointer->base, event->delta_x, event->delta_y);
    handle_motion_shared(state, event->time_msec);
}

static void
handle_motion_absolute(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_pointer_motion_absolute_event *event = data;
    struct state *state = state_get();

    double lx, ly;
    wlr_cursor_absolute_to_layout_coords(state->cursor.wlr_cursor, &event->pointer->base, event->x, event->y, &lx, &ly);

    // TODO: when relative
    // double dx = lx - state->cursor.wlr_cursor->x;
    // double dy = ly - state->cursor.wlr_cursor->y;

    // wlr_relative_pointer_manager_v1_send_relative_motion(state->relative_pointer_manager, state->seat.wlr_seat,
    //         (u64)event->time_msec * 1000, dx, dy, dx, dy);

    wlr_cursor_warp_absolute(state->cursor.wlr_cursor, &event->pointer->base, event->x, event->y);
    handle_motion_shared(state, event->time_msec);
}

static void
handle_button(struct wl_listener *listener, void *data) {
    UNUSED(listener);
    struct wlr_pointer_button_event *event = data;
    struct state *state = state_get();

    // TODO: when pointer binds
    // u32 modifiers = state->active_keyboard ? wlr_keyboard_get_modifiers(state->active_keyboard->wlr_keyboard) : 0;

    // struct keybind *k;
    // wl_list_for_each(k, &server.config->pointer_keybinds, link) {
    //     if(!k->initialized)
    //         continue;
    //
    //     if(k->active && k->stop && event->button == k->key && event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
    //         k->active = false;
    //         k->stop(k->args);
    //         return;
    //     }
    //
    //     if(modifiers == k->modifiers && event->button == k->key && event->state == WL_POINTER_BUTTON_STATE_PRESSED) {
    //         k->active = true;
    //         k->action(k->args);
    //         return;
    //     }
    // }

    // notify the client with pointer focus that a button press has occurred
    wlr_seat_pointer_notify_button(state->seat.wlr_seat, event->time_msec, event->button, event->state);

    // TODO: recheck these deps and maybe not infrom client if consumed?
    if(event->state == WL_POINTER_BUTTON_STATE_RELEASED && state->operation) {
        operation_stop_whatever(state);
    }
}

static void
handle_axis(struct wl_listener *listener, void *data) {
    UNUSED(listener);
    struct wlr_pointer_axis_event *event = data;
    struct state *state = state_get();

    wlr_seat_pointer_notify_axis(state->seat.wlr_seat, event->time_msec, event->orientation, event->delta,
            event->delta_discrete, event->source, event->relative_direction);
}

static void
handle_frame(struct wl_listener *listener, void *data) {
    UNUSED(listener), UNUSED(data);

    struct state *state = state_get();
    wlr_seat_pointer_notify_frame(state->seat.wlr_seat);
}

void
handle_request_set_shape(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_cursor_shape_manager_v1_request_set_shape_event *event = data;
    struct state *state = state_get();

    struct wlr_seat_client *focused_client = state->seat.wlr_seat->pointer_state.focused_client;
    if(focused_client == event->seat_client) {
        const char *name = wlr_cursor_shape_v1_name(event->shape);
        wlr_cursor_set_xcursor(state->cursor.wlr_cursor, state->cursor.xcursor_mgr, name);
    }
}

void
cursor_set_theme(struct cursor *cursor, char *theme, int size) {
    if(cursor->xcursor_mgr) {
        wlr_xcursor_manager_destroy(cursor->xcursor_mgr);
        cursor->xcursor_mgr = NULL;
    }

    cursor->xcursor_mgr = wlr_xcursor_manager_create(theme, size);

    // also set envirorment variables
    char cursor_size[8];
    snprintf(cursor_size, sizeof(cursor_size), "%d", size);
    cursor_size[7] = 0;

    setenv("XCURSOR_SIZE", cursor_size, true);
    setenv("XCURSOR_THEME", theme, true);
}

void
cursor_init(struct cursor *cursor, struct wlr_output_layout *output_layout) {
    cursor->wlr_cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(cursor->wlr_cursor, output_layout);

    // TODO: is there a default theme with no manager or we need to set it?
    // cursor_set_theme();

    cursor->motion.notify = handle_motion;
    wl_signal_add(&cursor->wlr_cursor->events.motion, &cursor->motion);

    cursor->motion_absolute.notify = handle_motion_absolute;
    wl_signal_add(&cursor->wlr_cursor->events.motion_absolute, &cursor->motion_absolute);

    cursor->button.notify = handle_button;
    wl_signal_add(&cursor->wlr_cursor->events.button, &cursor->button);

    cursor->axis.notify = handle_axis;
    wl_signal_add(&cursor->wlr_cursor->events.axis, &cursor->axis);

    cursor->frame.notify = handle_frame;
    wl_signal_add(&cursor->wlr_cursor->events.frame, &cursor->frame);

    cursor->cursor_shape_manager =
            wlr_cursor_shape_manager_v1_create(output_layout->display, COMPHY_CURSOR_SHAPE_VERSION);

    cursor->request_set_shape.notify = handle_request_set_shape;
    wl_signal_add(&cursor->cursor_shape_manager->events.request_set_shape, &cursor->request_set_shape);
}

void
cursor_deinit(struct cursor *cursor) {
    wl_list_remove(&cursor->motion.link);
    wl_list_remove(&cursor->motion_absolute.link);
    wl_list_remove(&cursor->button.link);
    wl_list_remove(&cursor->axis.link);
    wl_list_remove(&cursor->frame.link);
    wl_list_remove(&cursor->request_set_shape.link);

    if(cursor->xcursor_mgr) {
        wlr_xcursor_manager_destroy(cursor->xcursor_mgr);
    }

    wlr_cursor_destroy(cursor->wlr_cursor);
}

struct output *
cursor_get_output(struct state *state) {
    struct wlr_output *wlr_output =
            wlr_output_layout_output_at(state->output_layout, state->cursor.wlr_cursor->x, state->cursor.wlr_cursor->y);

    if(!wlr_output) {
        wlr_log(WLR_ERROR, "seems like the pointer is not on any output, bug?");
        return NULL;
    }

    struct output *output = wlr_output->data;
    return output;
}

void
cursor_warp_focused_toplevel(struct state *state) {
    struct toplevel *toplevel = state->focused_toplevel;
    if(!toplevel) {
        return;
    }

    wlr_cursor_warp(state->cursor.wlr_cursor, NULL, toplevel->current.x + toplevel->current.width / 2.0f,
            toplevel->current.y + toplevel->current.height / 2.0);
    cursor_focus(state, time_now_ms(), false);
}

void
cursor_warp_output(struct state *state, struct output *output) {
    wlr_cursor_warp(state->cursor.wlr_cursor, NULL, output->full_area.x + output->full_area.width / 2.0f,
            output->full_area.y + output->full_area.height / 2.0);
    cursor_focus(state, time_now_ms(), false);
}
