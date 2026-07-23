#ifndef LAYER_H
#define LAYER_H

#include <stdbool.h>
#include <wlr/types/wlr_layer_shell_v1.h>

#include "output.h"
#include "view.h"

struct layer {
    struct wlr_layer_surface_v1 *wlr_layer;
    struct wlr_scene_layer_surface_v1 *scene;

    enum view view;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener new_popup;
    struct wl_listener destroy;

    struct wl_list link;
};

struct layer *
layer_create(struct wlr_layer_surface_v1 *wlr_layer);

void
layer_surfaces_commit(struct output *output);

void
layer_focus(struct layer *layer);

void
layer_under_fullscreen_set_enabled(struct output *output, bool enable);

#endif
