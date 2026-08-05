#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <stdint.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "color.h"
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
    bool needs_centering, needs_initial_enable;

    // for the toplevel we create a scene tree, which contains the whole toplevel presentation on the screen: the
    // decorations and the content. when the toplevel is dirty we create the last snapshot of the content tree and
    // keep it in the snapshot_tree until the client commits the new content coresponding to the desired geometry
    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_tree *content_tree;
    struct wlr_scene_rect *border;

    struct wlr_scene_tree *snapshot_tree;
    struct wl_event_source *transaction_schedule_idle;

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
toplevel_configure(struct state *state, struct toplevel *toplevel, struct wlr_box *box);

void
toplevel_set_fullscreen(struct state *state, struct toplevel *toplevel, bool set);

struct output *
toplevel_float_largest_output_intersection(struct state *state, struct toplevel *toplevel);

void
toplevel_move_to_workspace(struct state *state, struct toplevel *toplevel, struct workspace *workspace);

u32
toplevel_get_corner_closest_to(struct toplevel *toplevel, int x, int y);

void
toplevel_set_border_color(struct toplevel *toplevel, color_t color);

#endif
