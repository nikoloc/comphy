#ifndef SEAT_H
#define SEAT_H

#include <wlr/types/wlr_seat.h>

struct seat {
    struct wlr_seat *wlr_seat;

    // TODO: move to operation
    bool drag_active;

    // TODO: may move to scene and add helpers for it
    struct wlr_scene_tree *drag_icon_tree;

    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_listener request_drag;
    struct wl_listener start_drag;
    struct wl_listener destroy_drag;
};

void
seat_init(struct seat *seat, struct wl_display *display);

void
seat_deinit(struct seat *seat);

struct state;

void
seat_inform_capabilities(struct state *state);

#endif
