#ifndef LOCK_H
#define LOCK_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_session_lock_v1.h>

#include "view.h"

struct lock_mgr {
    struct wlr_session_lock_manager_v1 *wlr_lock_mgr;
    struct lock *lock;

    struct wl_listener new_lock;
};

struct lock {
    struct wlr_session_lock_v1 *wlr_lock;
    bool locked;

    struct wl_list surfaces;

    struct wl_listener new_surface;
    struct wl_listener unlock;
    struct wl_listener destroy;
};

struct lock_surface {
    struct wlr_session_lock_surface_v1 *wlr_lock_surface;
    struct wlr_scene_tree *scene;
    enum view view;

    struct lock *lock;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;

    struct wl_list link;
};

struct lock *
lock_create(struct wlr_session_lock_v1 *wlr_lock);

void
lock_surface_focus(struct state *state, struct lock_surface *lock_surface);

void
lock_mgr_init(struct lock_mgr *mgr, struct wl_display *display);

#endif
