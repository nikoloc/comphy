#ifndef DECORATION_H
#define DECORATION_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_server_decoration.h>

struct decoration {
    struct wlr_xdg_decoration_manager_v1 *xdg_decoration_manager;
    struct wl_listener new_decoration;

    struct wlr_server_decoration_manager *kde_decoration_manager;
};

void
decoration_init(struct decoration *decoration, struct wl_display *display);

void
decoration_deinit(struct decoration *decoration);

#endif
