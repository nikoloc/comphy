#ifndef BACKEND_H
#define BACKEND_H

#include <wlr/backend.h>
#include <wlr/backend/session.h>

struct backend {
    struct wlr_backend *wlr_backend;
    struct wlr_session *wlr_session;

    struct wl_list outputs;

    struct wl_listener new_output;
    struct wl_listener new_input;
};

struct backend *
backend_create(struct wl_display *display);

void
backend_destroy(struct backend *backend);

bool
backend_switch_vt_session(struct backend *backend, int vt);

#endif
