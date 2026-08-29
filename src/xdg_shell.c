#include "xdg_shell.h"

#include "comphy.h"
#include "popup.h"
#include "state.h"
#include "util/macros.h"
#include "workspace.h"

static void
handle_new_toplevel(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_xdg_toplevel *wlr_toplevel = data;
    struct state *state = state_get();

    toplevel_create(state, wlr_toplevel);
}
static void
handle_new_popup(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_xdg_popup *wlr_popup = data;
    struct state *state = state_get();

    popup_create(state, wlr_popup);
}

static void
handle_request_activate(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_xdg_activation_v1_request_activate_event *event = data;
    struct state *state = state_get();

    enum view *view = view_root_parent_of_surface(event->surface);
    if(!view || *view != VIEW_TOPLEVEL) {
        return;
    }

    struct toplevel *toplevel = view_get_toplevel(view);
    if(toplevel->wlr_toplevel->base->surface->mapped) {
        workspace_set_active(state, toplevel->workspace, true);
        toplevel_focus(state, toplevel, true);
    }
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
}

void
xdg_shell_deinit(struct xdg_shell *shell) {
    wl_list_remove(&shell->new_toplevel.link);
    wl_list_remove(&shell->new_popup.link);

    wl_list_remove(&shell->request_activate.link);
}
