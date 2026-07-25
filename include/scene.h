#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include <wlr/types/wlr_scene.h>

struct scene {
    struct wlr_scene *wlr_scene;
    struct wlr_scene_output_layout *scene_layout;

    struct scene_trees {
        struct wlr_scene_tree *background;
        struct wlr_scene_tree *bottom;
        struct wlr_scene_tree *floats;
        struct wlr_scene_tree *tiled;
        struct wlr_scene_tree *top;
        struct wlr_scene_tree *fullscreen;
        struct wlr_scene_tree *grab;
        struct wlr_scene_tree *overlay;
        struct wlr_scene_tree *lock;
    } trees;
};

void
scene_init(struct scene *scene, struct wlr_output_layout *output_layout);

void
scene_deinit(struct scene *scene);

#endif
