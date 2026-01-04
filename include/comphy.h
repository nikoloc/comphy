#ifndef COMPHY_H
#define COMPHY_H

#include <wayland-server.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>

#include "backend.h"

struct g {
    struct wl_display *display;
    struct backend *backend;

    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    // TODO: move to our scene wrapper scene
    struct {
        struct wlr_scene_tree *background_tree;
        struct wlr_scene_tree *bottom_tree;
        struct wlr_scene_tree *tiled_tree;
        struct wlr_scene_tree *floating_tree;
        struct wlr_scene_tree *top_tree;
        struct wlr_scene_tree *fullscreen_tree;
        struct wlr_scene_tree *overlay_tree;
        struct wlr_scene_tree *session_lock_tree;
        struct wlr_scene_tree *drag_icon_tree;
    } trees;

    struct wl_list outputs;
    struct wlr_output_layout *output_layout;
    struct wl_listener new_output;
};

// piece of global state
extern struct g g;

#endif
