#include "seat.h"

#include <wlr/types/wlr_data_device.h>

#include "state.h"
#include "util/ints.h"
#include "util/macros.h"

static void
handle_request_cursor(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct state *state = state_get();

    if(state->operation && state->operation_server_inited) {
        return;
    }

    // we do it based on pointer focus
    struct wlr_seat_client *focused_client = state->seat.wlr_seat->pointer_state.focused_client;
    if(focused_client == event->seat_client) {
        wlr_cursor_set_surface(state->cursor.wlr_cursor, event->surface, event->hotspot_x, event->hotspot_y);
    }
}

static void
handle_request_set_selection(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_seat_request_set_selection_event *event = data;
    struct state *state = state_get();
    wlr_seat_set_selection(state->seat.wlr_seat, event->source, event->serial);
}

void
seat_init(struct seat *seat, struct wl_display *display) {
    seat->wlr_seat = wlr_seat_create(display, "seat0");

    seat->request_cursor.notify = handle_request_cursor;
    wl_signal_add(&seat->wlr_seat->events.request_set_cursor, &seat->request_cursor);

    seat->request_set_selection.notify = handle_request_set_selection;
    wl_signal_add(&seat->wlr_seat->events.request_set_selection, &seat->request_set_selection);

    // server.drag_icon_tree = wlr_scene_tree_create(&server.scene->tree);
    // wlr_scene_node_set_enabled(&server.drag_icon_tree->node, false);

    // TODO: add dnd
    // server.request_drag.notify = server_handle_request_drag;
    // wl_signal_add(&server.seat->events.request_start_drag, &server.request_drag);
    //
    // server.request_start_drag.notify = server_handle_request_start_drag;
    // wl_signal_add(&server.seat->events.start_drag, &server.request_start_drag);

    // TODO: check this
    // server.request_destroy_drag.notify = server_handle_destroy_drag;
}

void
seat_deinit(struct seat *seat) {
    wl_list_remove(&seat->request_cursor.link);
    wl_list_remove(&seat->request_set_selection.link);

    // TODO: i think we dont need this since it has the display destroy private listener
    // wlr_seat_destroy(seat->wlr_seat);
}

void
seat_inform_capabilities(struct state *state) {
    u32 caps = WL_SEAT_CAPABILITY_POINTER;
    if(!wl_list_empty(&state->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }

    wlr_seat_set_capabilities(state->seat.wlr_seat, caps);
}

enum view *
seat_get_pointer_focused(struct state *state) {
    struct wlr_surface *surface = state->seat.wlr_seat->pointer_state.focused_surface;
    if(!surface) {
        return NULL;
    }

    return view_root_parent_of_surface(surface);
}
