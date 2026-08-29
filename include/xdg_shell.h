#ifndef XDG_SHELL
#define XDG_SHELL

#include <wlr/types/wlr_xdg_activation_v1.h>
#include <wlr/types/wlr_xdg_shell.h>

struct xdg_shell {
    struct wlr_xdg_shell *wlr_xdg_shell;
    struct wl_listener new_toplevel;
    struct wl_listener new_popup;

    struct wlr_xdg_activation_v1 *xdg_activation;
    struct wl_listener request_activate;
};

void
xdg_shell_init(struct xdg_shell *shell, struct wl_display *display);

void
xdg_shell_deinit(struct xdg_shell *shell);

#endif
