#include "scene.h"

#include <wlr/types/wlr_scene.h>

#include "alloc.h"

struct scene *
scene_create(struct backend *backend) {
    struct scene *scene = alloc(sizeof(*scene));
    scene->wlr_scene = wlr_scene_create();

    return scene;
}

void
scene_destroy(struct scene *scene) {
    free(scene);
}
