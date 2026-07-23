#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <wayland-server-protocol.h>

#include "config.h"
#include "output.h"
#include "toplevel.h"

struct mwc_workspace {
    int index;
    struct output *output;
    struct workspace_config *config;

    struct toplevel *master;
    struct wl_list slaves, floats;
    struct toplevel *fullscreen;

    struct wl_list link;
};

void
workspace_create_for_output(struct mwc_output *output, struct workspace_config *config);

void
change_workspace(struct mwc_workspace *workspace, bool keep_focus);

void
toplevel_move_to_workspace(struct mwc_toplevel *toplevel, struct mwc_workspace *workspace);

struct mwc_toplevel *
workspace_find_closest_floating_toplevel(struct mwc_workspace *workspace, enum mwc_direction side);

#endif
