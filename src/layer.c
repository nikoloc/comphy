#include "layer.h"

#include <math.h>
#include <wayland-util.h>
#include <wlr/types/wlr_fractional_scale_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "config.h"
#include "layout.h"
#include "list_helpers.h"
#include "output.h"
#include "popup.h"
#include "toplevel.h"
#include "util/macros.h"
#include "util/memory.h"
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include "workspace.h"

static struct wlr_scene_tree *
get_scene(struct state *state, enum zwlr_layer_shell_v1_layer layer) {
    switch(layer) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND: {
            return state->scene.trees.background;
        }
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM: {
            return state->scene.trees.bottom;
        }
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP: {
            return state->scene.trees.top;
        }
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY: {
            return state->scene.trees.overlay;
        }
    }
}

static struct wl_list *
get_list(struct output *output, enum zwlr_layer_shell_v1_layer layer) {
    switch(layer) {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND: {
            return &output->layers.background;
        }
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM: {
            return &output->layers.bottom;
        }
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP: {
            return &output->layers.top;
        }
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY: {
            return &output->layers.overlay;
        }
    }
}

static void
handle_commit(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct layer *layer = CONTAINER_OF(listener, struct layer, commit);
    struct state *state = state_get();

    if(!layer->wlr_layer->initialized) {
        return;
    }

    struct output *output = layer->wlr_layer->output->data;

    if(layer->wlr_layer->initial_commit) {
        // we need a copy since this is a dummy
        struct wlr_box dummy = output->full_area;
        wlr_scene_layer_surface_v1_configure(layer->scene_tree, &dummy, &dummy);
        return;
    }

    u32 committed = layer->wlr_layer->current.committed;
    enum zwlr_layer_shell_v1_layer shell_layer = layer->wlr_layer->current.layer;
    if(committed & WLR_LAYER_SURFACE_V1_STATE_LAYER) {
        if(layer->wlr_layer->surface->mapped) {
            // if the layer has been changed we respect it
            struct wl_list *list = get_list(output, shell_layer);
            wl_list_remove(&layer->link);
            wl_list_insert(list, &layer->link);
        }

        struct wlr_scene_tree *scene = get_scene(state, shell_layer);
        wlr_scene_node_reparent(&layer->scene_tree->tree->node, scene);
    }

    if(committed) {
        layers_arrange(state, output);
    }
}

static void
handle_map(struct wl_listener *listener, void *data) {
    UNUSED(data);
    struct layer *layer = CONTAINER_OF(listener, struct layer, map);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "layer '%p' mapped", (void *)layer);

    struct output *output = layer->wlr_layer->output->data;

    // insert it into a list
    enum zwlr_layer_shell_v1_layer shell_layer = layer->wlr_layer->pending.layer;
    struct wl_list *list = get_list(output, shell_layer);
    wl_list_insert(list, &layer->link);

    wlr_scene_node_raise_to_top(&layer->scene_tree->tree->node);
    layers_arrange(state, output);
    layer_focus(state, layer, true);
}

static void
handle_unmap(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct layer *layer = CONTAINER_OF(listener, struct layer, unmap);
    struct state *state = state_get();

    wlr_log(WLR_DEBUG, "layer '%p' unmapped", (void *)layer);

    wl_list_remove(&layer->link);

    struct output *output = layer->wlr_layer->output->data;

    if(!output) {
        struct wl_list *first = wl_list_first(&state->outputs);
        if(!first) {
            return;
        }

        output = CONTAINER_OF(first, struct output, link);
    }

    if(layer == state->focused_layer) {
        state->is_exclusive = false;

        output_focus(state, output, true);
    }

    layers_arrange(state, output);
}

void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct layer *layer = CONTAINER_OF(listener, struct layer, destroy);

    wlr_log(WLR_DEBUG, "layer '%p' destroyed", (void *)layer);

    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);
    wl_list_remove(&layer->commit.link);
    wl_list_remove(&layer->new_popup.link);
    wl_list_remove(&layer->destroy.link);

    FREE(layer);
}

static void
handle_new_popup(struct wl_listener *listener, void *data) {
    struct layer *layer = CONTAINER_OF(listener, struct layer, new_popup);
    struct wlr_xdg_popup *xdg_popup = data;

    wlr_log(WLR_DEBUG, "new layer popup for layer '%p'", (void *)layer);

    struct popup *popup = xdg_popup->base->data;

    struct wlr_scene_tree *parent_tree = layer->scene_tree->tree;
    popup->scene_tree = wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);

    popup->scene_tree->node.data = &popup->view;
}

void
layer_destroy(struct layer *layer) {
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);
    wl_list_remove(&layer->commit.link);
    wl_list_remove(&layer->new_popup.link);
    wl_list_remove(&layer->destroy.link);

    wl_list_remove(&layer->link);

    wlr_layer_surface_v1_destroy(layer->wlr_layer);
    FREE(layer);
}

struct layer *
layer_create(struct state *state, struct wlr_layer_surface_v1 *wlr_layer) {
    struct layer *layer = ALLOC(struct layer);
    layer->wlr_layer = wlr_layer;
    wlr_layer->data = layer;

    layer->view = VIEW_LAYER;

    if(!wlr_layer->output) {
        wlr_layer->output = state->active_workspace->output->wlr_output;
    }

    struct output *output = layer->wlr_layer->output->data;
    wlr_fractional_scale_v1_notify_scale(wlr_layer->surface, output->wlr_output->scale);
    wlr_surface_set_preferred_buffer_scale(wlr_layer->surface, ceil(output->wlr_output->scale));

    enum zwlr_layer_shell_v1_layer shell_layer = layer->wlr_layer->pending.layer;

    struct wlr_scene_tree *scene = get_scene(state, shell_layer);
    layer->scene_tree = wlr_scene_layer_surface_v1_create(scene, layer->wlr_layer);
    layer->scene_tree->tree->node.data = &layer->view;

    layer->commit.notify = handle_commit;
    wl_signal_add(&wlr_layer->surface->events.commit, &layer->commit);

    layer->map.notify = handle_map;
    wl_signal_add(&wlr_layer->surface->events.map, &layer->map);

    layer->unmap.notify = handle_unmap;
    wl_signal_add(&wlr_layer->surface->events.unmap, &layer->unmap);

    layer->new_popup.notify = handle_new_popup;
    wl_signal_add(&wlr_layer->events.new_popup, &layer->new_popup);

    layer->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_layer->events.destroy, &layer->destroy);

    return layer;
}

void
layer_focus(struct state *state, struct layer *layer, bool warp) {
    if(state->lock_mgr.lock) {
        return;
    }

    enum zwlr_layer_surface_v1_keyboard_interactivity keyboard_interactive =
            layer->wlr_layer->current.keyboard_interactive;

    if(keyboard_interactive == ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE ||
            (keyboard_interactive == ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND && state->is_exclusive)) {
        return;
    }

    view_unfocus(state);

    state->focused_layer = layer;
    state->is_exclusive = keyboard_interactive == ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE;

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(state->seat.wlr_seat);
    if(keyboard) {
        wlr_seat_keyboard_notify_enter(state->seat.wlr_seat, layer->wlr_layer->surface, keyboard->keycodes,
                keyboard->num_keycodes, &keyboard->modifiers);
    }

    if(warp && state->config.cursor.warp) {
        cursor_warp_layer(state, layer);
    }
}

void
arrange_layer(struct output *output, enum zwlr_layer_shell_v1_layer layer, bool exclusive) {
    struct wl_list *list = get_list(output, layer);

    struct wlr_box full_area = output->full_area;

    struct layer *iter;
    wl_list_for_each_reverse(iter, list, link) {
        if((iter->wlr_layer->current.exclusive_zone > 0) != exclusive) {
            continue;
        }

        wlr_scene_layer_surface_v1_configure(iter->scene_tree, &full_area, &output->usable_area);
    }
}

void
layers_arrange(struct state *state, struct output *output) {
    output->usable_area = output->full_area;

    // first commit all the exclusive ones
    for(int i = 0; i < 4; i++) {
        arrange_layer(output, i, true);
    }

    // then all the others
    for(int i = 0; i < 4; i++) {
        arrange_layer(output, i, false);
    }

    struct workspace *iter;
    wl_list_for_each(iter, &output->workspaces, link) {
        layout_configure(state, iter);
    }
}

void
layers_arrange_all(struct state *state) {
    struct output *output;
    wl_list_for_each(output, &state->outputs, link) {
        layers_arrange(state, output);
    }
}
