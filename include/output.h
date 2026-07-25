#ifndef OUTPUT_H
#define OUTPUT_H

#include <wlr/types/wlr_output.h>

#include "state.h"

struct output {
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;
    struct wlr_box full_area, usable_area;

    struct wl_list workspaces;

    struct {
        struct wl_list background;
        struct wl_list bottom;
        struct wl_list top;
        struct wl_list overlay;
    } layer;

    struct workspace *active_workspace;

    struct wlr_scene_rect *lock_rect;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;

    struct wl_list link;
};

struct output *
output_create(struct state *state, struct wlr_output *wlr_output);

// give focus to some view on this workspace in the general focus order; does not handle workspace switching!
void
output_focus(struct state *state, struct output *output);

// struct wlr_box
// output_add_to_layout(struct output *output, struct output_config *config);
//
// bool
// output_initialize(struct wlr_output *output, struct output_config *config);

bool
output_transfer_existing_workspaces(struct state *state, struct output *output);

struct workspace *
output_find_owned_workspace(struct state *state, struct output *output);

// take better functions from comphy-old

bool
output_apply_preffered_mode(struct wlr_output *wlr_output, struct wlr_output_state *state);

struct output *
output_get_relative(struct state *state, struct output *output, enum wlr_direction direction);

void
output_warp_cursor(struct state *state, struct output *output);

void
output_move_workspaces(struct output *dest, struct output *src);

#endif
