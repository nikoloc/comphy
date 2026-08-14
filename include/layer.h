#ifndef LAYER_H
#define LAYER_H

#include <stdbool.h>
#include <wlr/types/wlr_layer_shell_v1.h>

#include "view.h"

struct layer {
    struct wlr_layer_surface_v1 *wlr_layer;
    struct wlr_scene_layer_surface_v1 *scene_tree;

    enum view view;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener new_popup;
    struct wl_listener destroy;

    struct wl_list link;
};

struct layer *
layer_create(struct state *state, struct wlr_layer_surface_v1 *wlr_layer);

void
layer_focus(struct state *state, struct layer *layer);

void
layer_unfocus(struct state *state);

void
layers_arrange(struct state *state, struct output *output);

void
layers_arrange_all(struct state *state);

#endif
