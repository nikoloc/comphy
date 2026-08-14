#ifndef CURSOR_H
#define CURSOR_H

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_cursor_shape_v1.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "layer.h"
#include "toplevel.h"
#include "util/ints.h"

struct cursor {
    struct wlr_cursor *wlr_cursor;
    struct wlr_xcursor_manager *xcursor_mgr;

    struct wl_listener motion;
    struct wl_listener motion_absolute;
    struct wl_listener button;
    struct wl_listener axis;
    struct wl_listener frame;

    struct wlr_cursor_shape_manager_v1 *cursor_shape_manager;
    struct wl_listener request_set_shape;
};

void
cursor_init(struct cursor *cursor, struct wlr_output_layout *output_layout);

void
cursor_deinit(struct cursor *cursor);

struct state;

struct output *
cursor_get_output(struct state *state);

void
cursor_set_image(struct state *state, char *image);

void
cursor_focus(struct state *state, u32 time_ms, bool handle_keyboard_focus);

// this needs to take the cursor since it is also called from the init function
void
cursor_set_theme(struct cursor *cursor, char *theme, int size);

void
cursor_warp_focused_toplevel(struct state *state);

void
cursor_warp_output(struct state *state, struct output *output);

void
cursor_warp_toplevel(struct state *state, struct toplevel *toplevel);

void
cursor_warp_layer(struct state *state, struct layer *layer);

#endif
