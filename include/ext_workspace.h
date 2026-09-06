#ifndef EXT_WORKSPACE_H
#define EXT_WORKSPACE_H

#include <wayland-server-protocol.h>
#include <wlr/types/wlr_ext_workspace_v1.h>

struct ext_workspace_mgr {
    struct wlr_ext_workspace_manager_v1 *wlr_mgr;

    struct wl_listener commit;
};

void
ext_workspace_mgr_init(struct ext_workspace_mgr *mgr, struct wl_display *display);

void
ext_workspace_mgr_deinit(struct ext_workspace_mgr *mgr);

#endif
