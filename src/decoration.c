#include "decoration.h"

#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/util/log.h>

#include "state.h"
#include "util/macros.h"

static void
handle_new_decoration(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_xdg_toplevel_decoration_v1 *decoration = data;
    struct state *state = state_get();

    enum wlr_xdg_toplevel_decoration_v1_mode mode = state->config.csd ? WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE
                                                                      : WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
    wlr_xdg_toplevel_decoration_v1_set_mode(decoration, mode);
}

void
decoration_init(struct decoration *decoration, struct wl_display *display) {
    decoration->xdg_decoration_manager = wlr_xdg_decoration_manager_v1_create(display);

    decoration->new_decoration.notify = handle_new_decoration;
    wl_signal_add(&decoration->xdg_decoration_manager->events.new_toplevel_decoration, &decoration->new_decoration);

    decoration->kde_decoration_manager = wlr_server_decoration_manager_create(display);
    // we default to server side decorations, note that this can be changed by the comphyctl call
    wlr_server_decoration_manager_set_default_mode(decoration->kde_decoration_manager,
            WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
}

void
decoration_deinit(struct decoration *decoration) {
    wl_list_remove(&decoration->new_decoration.link);
}

void
decoration_update(struct decoration *decoration, bool csd) {
    enum wlr_xdg_toplevel_decoration_v1_mode mode =
            csd ? WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE : WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;

    struct wlr_xdg_toplevel_decoration_v1 *iter;
    wl_list_for_each(iter, &decoration->xdg_decoration_manager->decorations, link) {
        wlr_xdg_toplevel_decoration_v1_set_mode(iter, mode);
    }

    enum wlr_server_decoration_manager_mode kde_mode =
            csd ? WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT : WLR_SERVER_DECORATION_MANAGER_MODE_SERVER;
    wlr_server_decoration_manager_set_default_mode(decoration->kde_decoration_manager, kde_mode);
}
