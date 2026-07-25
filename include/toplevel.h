#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <stdint.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "util/ints.h"
#include "view.h"

enum toplevel_state {
    TOPLEVEL_STATE_TILED,
    TOPLEVEL_STATE_FLOAT,
    TOPLEVEL_STATE_FULLSCREEN,
};

struct toplevel {
    struct wlr_xdg_toplevel *wlr_toplevel;
    struct workspace *workspace;

    enum view view;

    // `prev_state` and `prev_geometry` are used to return the fullscreen toplevel to a previous state
    enum toplevel_state state, prev_state;
    struct wlr_box prev_geometry;

    bool is_resizing;

    u32 configure_serial;
    bool is_dirty;

    // TODO: figure out what to do with this
    double inactive_opacity;
    double active_opacity;

    struct wlr_box current, pending;

    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_rect *scene_border;

    struct wlr_foreign_toplevel_handle_v1 *foreign_toplevel_handle;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
    struct wl_listener set_app_id;
    struct wl_listener set_title;

    struct wl_list link;
};

struct state;

struct toplevel *
toplevel_create(struct state *state, struct wlr_xdg_toplevel *wlr_toplevel);

void
toplevel_focus(struct state *state, struct toplevel *toplevel);

void
toplevel_set_fullscreen(struct state *state, struct toplevel *toplevel, bool set);

struct output *
toplevel_float_largest_output_intersection(struct state *state, struct toplevel *toplevel);

#endif
