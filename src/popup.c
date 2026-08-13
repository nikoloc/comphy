#include "popup.h"

#include <stdlib.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "layer.h"
#include "toplevel.h"
#include "util/macros.h"
#include "util/memory.h"
#include "view.h"
#include "workspace.h"

static void
handle_commit(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct popup *popup = CONTAINER_OF(listener, struct popup, commit);

    if(!popup->wlr_popup->base->initialized) {
        return;
    }

    if(popup->wlr_popup->base->initial_commit) {
        enum view *parent = popup_get_root_parent(popup);

        if(!parent) {
            wlr_xdg_surface_schedule_configure(popup->wlr_popup->base);
            return;
        }

        struct wlr_box box;
        if(*parent == VIEW_TOPLEVEL) {
            struct toplevel *toplevel = CONTAINER_OF(parent, struct toplevel, view);

            // in order for it to not cover the shell, we use usable area
            box = toplevel->workspace->output->usable_area;
            box.x -= toplevel->scene_tree->node.x;
            box.y -= toplevel->scene_tree->node.y;

        } else {
            // NOTE: assume layer surface?
            struct layer *layer = CONTAINER_OF(parent, struct layer, view);
            struct output *output = layer->wlr_layer->output->data;

            box = output->full_area;
            box.x -= layer->scene_tree->tree->node.x;
            box.y -= layer->scene_tree->tree->node.y;
        }

        wlr_xdg_popup_unconstrain_from_box(popup->wlr_popup, &box);
    }
}

static void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct popup *popup = CONTAINER_OF(listener, struct popup, destroy);

    wl_list_remove(&popup->commit.link);
    wl_list_remove(&popup->destroy.link);

    free(popup);
}

struct popup *
popup_create(struct state *state, struct wlr_xdg_popup *wlr_popup) {
    UNUSED(state);

    struct popup *popup = ALLOC(struct popup);
    popup->wlr_popup = wlr_popup;
    wlr_popup->base->data = popup;

    popup->view = VIEW_POPUP;

    if(wlr_popup->parent) {
        struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(wlr_popup->parent);
        if(parent) {
            struct wlr_scene_tree *parent_tree;
            if(parent->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
                struct toplevel *toplevel = parent->data;
                parent_tree = toplevel->content_tree;
            } else {
                // another popup
                struct popup *popup = parent->data;
                parent_tree = popup->scene_tree;
            }

            popup->scene_tree = wlr_scene_xdg_surface_create(parent_tree, wlr_popup->base);
            popup->scene_tree->node.data = &popup->view;
        }
    }

    popup->commit.notify = handle_commit;
    wl_signal_add(&wlr_popup->base->surface->events.commit, &popup->commit);

    popup->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_popup->events.destroy, &popup->destroy);

    return popup;
}

enum view *
popup_get_root_parent(struct popup *popup) {
    struct wlr_scene_tree *tree = popup->scene_tree;

    enum view *view = tree->node.data;
    while(!view || *view == VIEW_POPUP) {
        tree = tree->node.parent;
        view = tree->node.data;
    }

    return view;
}
