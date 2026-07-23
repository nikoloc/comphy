#ifndef POPUP_H
#define POPUP_H

#include <wlr/types/wlr_xdg_shell.h>

struct popup {
    struct wlr_xdg_popup *xdg_popup;
    enum view_type view_type;

    struct wlr_scene_tree *scene;

    struct wl_listener commit;
    struct wl_listener destroy;
};

struct popup *
popup_create(struct wlr_xdg_popup *wlr_popup);

enum view_type *
popup_get_root_parent(struct popup *popup);

#endif
