#include "scene.h"

void
scene_init(struct scene *scene, struct wlr_output_layout *output_layout) {
    scene->wlr_scene = wlr_scene_create();
    // TODO: fixme
    scene->scene_layout = wlr_scene_attach_output_layout(scene->wlr_scene, output_layout);

    // initialize them in the correct order
    scene->trees.background = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.bottom = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.tiled = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.floats = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.top = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.fullscreen = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.overlay = wlr_scene_tree_create(&scene->wlr_scene->tree);
    scene->trees.lock = wlr_scene_tree_create(&scene->wlr_scene->tree);
}

void
scene_deinit(struct scene *scene) {
    // destroying the root node destroys the whole scene graph
    wlr_scene_node_destroy(&scene->wlr_scene->tree.node);
}
