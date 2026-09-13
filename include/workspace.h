#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <wayland-server-protocol.h>
#include <wlr/types/wlr_ext_workspace_v1.h>

#include "output.h"
#include "toplevel.h"

struct workspace {
    int idx;

    struct output *output;
    // in case this workspaces output gets unplugged and then plugged again we use this info to return it to its
    // original output
    char *original_output_name;

    struct toplevel *master;
    struct wl_list slaves, floats;
    struct toplevel *fullscreen;

    struct wlr_ext_workspace_handle_v1 *ext_workspace;

    struct wl_list link;
};

struct workspace *
workspace_create(struct state *state, struct output *output, int idx);

void
workspace_destroy(struct state *state, struct workspace *workspace, bool output_is_destroying);

void
workspace_set_active(struct state *state, struct workspace *workspace, bool keep_focus);

void
workspace_show_toplevels(struct workspace *workspace, bool show);

struct workspace *
workspace_find_by_idx(struct state *state, int idx);

bool
workspace_is_presented(struct workspace *workspace);

#endif
