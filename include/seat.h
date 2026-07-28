#ifndef SEAT_H
#define SEAT_H

#include <wlr/types/wlr_seat.h>

struct seat {
    struct wlr_seat *wlr_seat;

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
