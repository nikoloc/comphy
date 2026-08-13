#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <wayland-server-protocol.h>

#include "config.h"
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

    bool has_dirty;
    struct wl_event_source *transaction_time_out;
    struct wl_event_source *transaction_schedule;
    // when the toplevel unmaps we keep its last frame until the transaction finishes. such toplevel is flagged and kept
    // here until the transaction is done, and is then destroyed for good.
    struct wl_list ghosts;

    struct wl_list link;
};

struct workspace *
workspace_create(struct state *state, struct output *output, int idx);

void
workspace_destroy(struct state *state, struct workspace *workspace);

void
workspace_set_active(struct state *state, struct workspace *workspace);

void
workspace_show_toplevels(struct workspace *workspace, bool show);

struct workspace *
workspace_find_by_idx(struct state *state, int idx);

#endif
