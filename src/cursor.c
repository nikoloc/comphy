#include "cursor.h"

#include "comphy.h"
#include "state.h"
#include "util/macros.h"
#include "wlr/util/log.h"

static void
handle_motion(struct wl_listener *listener, void *data) {
}

static void
handle_motion_absolute(struct wl_listener *listener, void *data) {
}

static void
handle_button(struct wl_listener *listener, void *data) {
}

static void
handle_axis(struct wl_listener *listener, void *data) {
}

static void
handle_frame(struct wl_listener *listener, void *data) {
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
cursor_init(struct cursor *cursor, struct wlr_output_layout *output_layout) {
    cursor->wlr_cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(cursor->wlr_cursor, output_layout);

    // TODO: figure out how we should handle the default cursor_theme
    // cursor->xcursor_mgr = wlr_xcursor_manager_create(server.config->cursor_theme, server.config->cursor_size);

    // TODO: move this to the post init part, or ipc
    // char cursor_size[8];
    // snprintf(cursor_size, sizeof(cursor_size), "%u", server.config->cursor_size);
    // cursor_size[7] = 0;
    // setenv("XCURSOR_SIZE", cursor_size, true);
    //
    // if(server.config->cursor_theme != NULL) {
    //     setenv("XCURSOR_THEME", server.config->cursor_theme, true);
    // }

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
