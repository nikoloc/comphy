#include "xdg_shell.h"

#include "comphy.h"

static void
handle_new_toplevel(struct wl_listener *listener, void *data) {
}

static void
handle_new_popup(struct wl_listener *listener, void *data) {
}

static void
handle_request_activate(struct wl_listener *listener, void *data) {
}

static void
handle_new_token(struct wl_listener *listener, void *data) {
}

void
xdg_shell_init(struct xdg_shell *shell, struct wl_display *display) {
    shell->wlr_xdg_shell = wlr_xdg_shell_create(display, COMPHY_XDG_SHELL_VERSION);

    shell->new_toplevel.notify = handle_new_toplevel;
    wl_signal_add(&shell->wlr_xdg_shell->events.new_toplevel, &shell->new_toplevel);

    shell->new_popup.notify = handle_new_popup;
    wl_signal_add(&shell->wlr_xdg_shell->events.new_popup, &shell->new_popup);

    shell->xdg_activation = wlr_xdg_activation_v1_create(display);

    shell->request_activate.notify = handle_request_activate;
    wl_signal_add(&shell->xdg_activation->events.request_activate, &shell->request_activate);

    shell->new_token.notify = handle_new_token;
    wl_signal_add(&shell->xdg_activation->events.new_token, &shell->new_token);
}

void
xdg_shell_deinit(struct xdg_shell *shell) {
    wl_list_remove(&shell->new_toplevel.link);
    wl_list_remove(&shell->new_popup.link);

    wl_list_remove(&shell->request_activate.link);
    wl_list_remove(&shell->new_token.link);
}
