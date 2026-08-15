#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <stdint.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "color.h"
#include "transaction.h"
#include "util/ints.h"
#include "view.h"

enum toplevel_state {
    TOPLEVEL_STATE_TILED = 0,
    TOPLEVEL_STATE_FLOAT,
    TOPLEVEL_STATE_FULLSCREEN,
};

struct toplevel {
    struct wlr_xdg_toplevel *wlr_toplevel;
    struct workspace *workspace;

    enum view view;

    // `prev_state` are used to return the fullscreen toplevel to a previous state
    enum toplevel_state state, prev_state;

    u32 configure_serial;
    enum transaction_state transaction_state;

    struct wlr_box current, pending;
    // parametars of the last `toplevel_configure()` call that were request of this toplevel. we need those because even
    // tho the full box may not change between subsequent calls, the decoroations may, hence we compute wheather the
    // size has changed based on this parametar
    int requested_width, requested_height;
    bool needs_centering, needs_initial_enable;
    // in order for everything scene-related to be atomic, we need to keep a lot of retained logic and only apply it on
    // transaction commit. this makes for a lot of bool flags bellow
    bool has_border;
    // should only be updated by `toplevel_update_state()`
    bool needs_reparenting;

    // for the toplevel we create a scene tree, which contains the whole toplevel presentation on the screen: the
    // decorations and the content. when the toplevel is dirty we create the last snapshot of the content tree and
    // keep it in the `snapshot_tree` until the client commits the new content coresponding to the desired geometry
    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_tree *content_tree;
    struct wlr_scene_rect *border;

    struct wlr_scene_tree *snapshot_tree;
    bool is_ghost;
    bool is_destroyed;

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

// this function does not consider the layout, lists, positioning etc and is meant to be called once all of
// that has been done by wahtever logic was needed in that scenario. this function does some final adjustements
// for the transaction system and sends the 'hacks' to the toplevel based on state
void
toplevel_update_state(struct toplevel *toplevel, enum toplevel_state state);

void
toplevel_send_frame_done(struct toplevel *toplevel);

void
toplevel_finalize_destroy(struct toplevel *toplevel);

void
toplevel_raise(struct toplevel *toplevel);

#endif
