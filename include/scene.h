#ifndef SCENE_H
#define SCENE_H

#include "backend.h"

struct scene {
    struct wlr_scene *wlr_scene;
};

struct scene *
scene_create(struct backend *backend);

void
scene_destroy(struct scene *scene);

#endif
