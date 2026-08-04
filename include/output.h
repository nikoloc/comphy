#ifndef OUTPUT_H
#define OUTPUT_H

#include <wlr/types/wlr_output.h>

#include "state.h"

struct output_config {
    int x, y;
    int width, height, refresh;

    float scale;
};

struct output {
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;
    struct wlr_output_layout_output *output_layout_output;

    struct wlr_box full_area, usable_area;

    struct wl_list workspaces;

    struct {
        struct wl_list background;
        struct wl_list bottom;
        struct wl_list top;
        struct wl_list overlay;
    } layer;

    // when the output is created we create a dummy workspace for it to serve until the user creates a real workspace.
    // on the creation of the first real workspace we just repace the dummy one with the new one.
    struct workspace *active_workspace, *dummy_workspace;

    struct wlr_scene_rect *lock_rect;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;

    struct wl_list link;
};

struct output *
output_create(struct state *state, struct wlr_output *wlr_output);

void
output_configure(struct state *state, struct output *output, struct output_config *config);

// give focus to some view on this workspace in the general focus order; does not handle workspace switching!
void
output_focus(struct state *state, struct output *output);

struct output *
output_get_relative(struct state *state, struct output *output, enum wlr_direction direction);

struct output *
output_find_by_name(struct state *state, char *name);

// bool
// output_transfer_existing_workspaces(struct state *state, struct output *output);
//
// struct workspace *
// output_find_owned_workspace(struct state *state, struct output *output);

// take better functions from comphy-old

#endif
