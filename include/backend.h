#ifndef BACKEND_H
#define BACKEND_H

#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/render/allocator.h>

#include "util/ints.h"

struct backend {
    struct wlr_backend *wlr_backend;
    struct wlr_session *session;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wl_listener new_output;
    struct wl_listener new_input;
};

bool
backend_init(struct backend *backend, struct wl_display *display);

bool
backend_start(struct backend *backend);

bool
backend_change_vt(struct backend *backend, u32 vt);

void
backend_deinit(struct backend *backend);

#endif
