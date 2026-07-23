#include "decoration.h"

#include <wlr/types/wlr_xdg_decoration_v1.h>

#include "util/macros.h"

static void
handle_new_decoration(struct wl_listener *listener, void *data) {
    struct wlr_xdg_toplevel_decoration_v1 *decoration = data;

    TODO("implement");

    // wlr_xdg_toplevel_decoration_v1_set_mode(decoration, server.config->client_side_decorations
    //                                                             ? WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE
    //                                                             : WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

void
decoration_init(struct decoration *decoration, struct wl_display *display) {
    decoration->xdg_decoration_manager = wlr_xdg_decoration_manager_v1_create(display);

    decoration->new_decoration.notify = handle_new_decoration;
    wl_signal_add(&decoration->xdg_decoration_manager->events.new_toplevel_decoration, &decoration->new_decoration);

    decoration->kde_decoration_manager = wlr_server_decoration_manager_create(display);
    // TODO: move this into the ipc call
    // wlr_server_decoration_manager_set_default_mode(decoration->kde_decoration_manager,
    //         server.config->client_side_decorations ? WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT
    //                                                : WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
}

void
decoration_deinit(struct decoration *decoration) {
    wl_list_remove(&decoration->new_decoration.link);
}
