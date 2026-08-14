#include "layout.h"

#include <stdint.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>

#include "box_helpers.h"
#include "config.h"
#include "list_helpers.h"
#include "toplevel.h"
#include "util/macros.h"

void
layout_add(struct workspace *workspace, struct toplevel *toplevel) {
    if(!workspace->master) {
        workspace->master = toplevel;
    } else {
        // insert it onto the bottom of the stack of slaves
        wl_list_insert(workspace->slaves.prev, &toplevel->link);
    }
}

void
layout_remove(struct toplevel *toplevel) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_TILED);

    struct workspace *workspace = toplevel->workspace;
    if(toplevel == workspace->master) {
        // fix the layout by inserting new master
        struct wl_list *next_master = wl_list_last(&workspace->slaves);
        if(next_master) {
            workspace->master = CONTAINER_OF(next_master, struct toplevel, link);
            wl_list_remove(next_master);
        } else {
            workspace->master = NULL;
        }
    } else {
        // just remove from the list of slaves
        wl_list_remove(&toplevel->link);
    }
}

void
layout_configure(struct state *state, struct workspace *workspace) {
    if(workspace->fullscreen || !workspace->master) {
        return;
    }

    struct output *output = workspace->output;

    bool smart_gaps = state->config.gaps.smart && wl_list_empty(&workspace->slaves);

    int outer_gaps = smart_gaps ? 0 : state->config.gaps.outer;
    int inner_gaps = smart_gaps ? 0 : state->config.gaps.inner;
    float master_ratio = state->config.master_ratio;

    struct wlr_box full_area = output->usable_area;
    wlr_box_remove_gaps(&full_area, outer_gaps);

    int slave_count = wl_list_length(&workspace->slaves);

    struct wlr_box box = {
            .x = full_area.x,
            .y = full_area.y,
            .width = slave_count > 0 ? full_area.width * master_ratio : full_area.width,
            .height = full_area.height,
    };

    wlr_box_remove_gaps(&box, inner_gaps);
    toplevel_configure(state, workspace->master, &box);

    if(slave_count == 0) {
        return;
    }

    int i = 0;
    // in order to not have any gaps at the bottom of the stack, we give the last slave all the remaining height
    int acc_height = 0;
    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->slaves, link) {
        box = (struct wlr_box){
                .x = full_area.x + full_area.width * master_ratio,
                .y = full_area.y + i * full_area.height / slave_count,
                .width = full_area.width * (1 - master_ratio),
                .height = wl_list_is_last(&workspace->slaves, &iter->link) ? full_area.height - acc_height
                                                                           : full_area.height / slave_count,
        };

        acc_height += box.height;

        wlr_box_remove_gaps(&box, inner_gaps);
        toplevel_configure(state, iter, &box);
        i++;
    }
}

void
layout_reconfigure_all(struct state *state) {
    struct output *output;
    wl_list_for_each(output, &state->outputs, link) {
        struct workspace *workspace;
        wl_list_for_each(workspace, &output->workspaces, link) {
            layout_configure(state, workspace);
        }
    }
}

struct toplevel *
layout_toplevel_at(struct workspace *workspace, int x, int y) {
    // TODO
    return NULL;
}
