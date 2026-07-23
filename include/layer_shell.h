#ifndef LAYER_SHELL
#define LAYER_SHELL

#include <wlr/types/wlr_layer_shell_v1.h>

struct layer_shell {
    struct wlr_layer_shell_v1 *wlr_layer_shell;

    struct wl_listener new_layer;
};

void
layer_shell_init(struct layer_shell *shell, struct wl_display *display);

void
layer_shell_deinit(struct layer_shell *shell);

#endif
