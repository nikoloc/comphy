#ifndef VIEW_H
#define VIEW_H

#include <wlr/types/wlr_compositor.h>

#include "util/macros.h"

enum view {
    VIEW_TOPLEVEL,
    VIEW_POPUP,
    VIEW_LAYER,
    VIEW_LOCK_SURFACE,
};

enum view *
view_root_parent_of_surface(struct wlr_surface *surface);

enum view *
view_at(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);

// TODO: move to .c, or delete entirely
void *
view_from(enum view *type) {
    switch(*type) {
        case VIEW_TOPLEVEL:
            return CONTAINER_OF(type, struct toplevel, view);
        case VIEW_POPUP:
            return CONTAINER_OF(type, struct popup, view);
        case VIEW_LAYER:
            return CONTAINER_OF(type, struct layer, view);
        case VIEW_LOCK_SURFACE:
            return CONTAINER_OF(type, struct lock_surface, view);
    }
}

#endif
