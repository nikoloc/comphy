#include "view.h"

#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "layer.h"
#include "lock.h"
#include "popup.h"
#include "toplevel.h"
#include "util/macros.h"
#include "workspace.h"

enum view *
view_get_focused(struct state *state) {
    if(state->focused_lock) {
        return &state->focused_lock->view;
    }

    if(state->focused_layer) {
        return &state->focused_layer->view;
    }

    return &state->focused_toplevel->view;
}

struct output *
view_get_output(enum view *view) {
    switch(*view) {
        case VIEW_TOPLEVEL: {
            struct toplevel *toplevel = CONTAINER_OF(view, struct toplevel, view);
            return toplevel->workspace->output;
        }
        case VIEW_POPUP: {
            struct popup *popup = CONTAINER_OF(view, struct popup, view);
            // recursively find the parents output
            return view_get_output(popup_get_root_parent(popup));
        }
        case VIEW_LAYER: {
            struct layer *layer = CONTAINER_OF(view, struct layer, view);
            return layer->wlr_layer->output->data;
        }
        case VIEW_LOCK_SURFACE: {
            struct lock_surface *lock = CONTAINER_OF(view, struct lock_surface, view);
            return lock->wlr_lock_surface->output->data;
        }
    }

    UNREACHABLE();
}

static enum view *
climb_tree_to_root(struct wlr_scene_tree *tree) {
    enum view *view = tree->node.data;
    while(!view || *view == VIEW_POPUP) {
        tree = tree->node.parent;
        view = tree->node.data;
    }

    return view;
}

enum view *
view_root_parent_of_surface(struct wlr_surface *surface) {
    surface = wlr_surface_get_root_surface(surface);

    struct wlr_xdg_surface *xdg_surface = wlr_xdg_surface_try_from_wlr_surface(surface);
    if(xdg_surface) {
        if(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
            struct toplevel *toplevel = xdg_surface->data;
            return &toplevel->view;
        } else {
            struct popup *popup = xdg_surface->data;
            return popup_get_root_parent(popup);
        }
    }

    struct wlr_layer_surface_v1 *wlr_layer = wlr_layer_surface_v1_try_from_wlr_surface(surface);
    if(wlr_layer) {
        struct layer *layer = wlr_layer->data;
        return &layer->view;
    }

    struct wlr_session_lock_surface_v1 *wlr_lock_surface = wlr_session_lock_surface_v1_try_from_wlr_surface(surface);
    if(wlr_lock_surface) {
        struct lock_surface *lock_surface = wlr_lock_surface->data;
        return &lock_surface->view;
    }

    return NULL;
}

enum view *
view_at(struct state *state, double lx, double ly, struct wlr_surface **surface, double *sx, double *sy) {
    // TODO: handle snapshots
    struct wlr_scene_node *node = wlr_scene_node_at(&state->scene.wlr_scene->tree.node, lx, ly, sx, sy);
    if(!node) {
        return NULL;
    }

    if(node->type == WLR_SCENE_NODE_BUFFER) {
        // NOTE: this is from tinywl, could not we just climb the tree until we find something?
        struct wlr_scene_buffer *buffer = wlr_scene_buffer_from_node(node);
        struct wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(buffer);
        if(!scene_surface) {
            // should not happen tho, since we do not produce our buffers yet
            return NULL;
        }

        *surface = scene_surface->surface;
        // climb up the tree to find the toplevel, layer or lock surface
        return climb_tree_to_root(node->parent);
    } else if(node->type == WLR_SCENE_NODE_RECT) {
        // this must be the border or the lock rect; we dont care about the later
        struct wlr_scene_tree *tree = node->parent;

        if(tree->node.data) {
            // must be the border
            enum view *view = tree->node.data;
            ASSERT(*view == VIEW_TOPLEVEL);
            return view;
        }
    }

    return NULL;
}

void
view_focus(struct state *state, enum view *view) {
    switch(*view) {
        case VIEW_TOPLEVEL: {
            struct toplevel *toplevel = CONTAINER_OF(view, struct toplevel, view);
            toplevel_focus(state, toplevel);
            break;
        }
        case VIEW_POPUP: {
            struct popup *popup = CONTAINER_OF(view, struct popup, view);
            view_focus(state, popup_get_root_parent(popup));
            break;
        }
        case VIEW_LAYER: {
            struct layer *layer = CONTAINER_OF(view, struct layer, view);
            // layer_focus(state, layer);
            break;
        }
        case VIEW_LOCK_SURFACE: {
            struct lock_surface *lock = CONTAINER_OF(view, struct lock_surface, view);
            // lock_surface_focus(state, lock);
            break;
        }
    }
}

struct toplevel *
view_get_toplevel(enum view *view) {
    if(*view == VIEW_TOPLEVEL) {
        return CONTAINER_OF(view, struct toplevel, view);
    }

    return NULL;
}

struct popup *
view_get_popup(enum view *view) {
    if(*view == VIEW_POPUP) {
        return CONTAINER_OF(view, struct popup, view);
    }

    return NULL;
}

struct layer *
view_get_layer(enum view *view) {
    if(*view == VIEW_LAYER) {
        return CONTAINER_OF(view, struct layer, view);
    }

    return NULL;
}

struct lock_surface *
view_get_lock_surface(enum view *view) {
    if(*view == VIEW_LOCK_SURFACE) {
        return CONTAINER_OF(view, struct lock_surface, view);
    }

    return NULL;
}
