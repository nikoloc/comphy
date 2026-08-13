#include "layer_shell.h"

#include "comphy.h"
#include "layer.h"
#include "state.h"
#include "util/macros.h"

static void
handle_new_layer(struct wl_listener *listener, void *data) {
    UNUSED(listener);
    struct wlr_layer_surface_v1 *wlr_layer = data;
    struct state *state = state_get();

    layer_create(state, wlr_layer);
}

void
layer_shell_init(struct layer_shell *shell, struct wl_display *display) {
    shell->wlr_layer_shell = wlr_layer_shell_v1_create(display, COMPHY_LAYER_SHELL_VERSION);

    shell->new_layer.notify = handle_new_layer;
    wl_signal_add(&shell->wlr_layer_shell->events.new_surface, &shell->new_layer);
}

void
layer_shell_deinit(struct layer_shell *shell) {
    wl_list_remove(&shell->new_layer.link);
}
