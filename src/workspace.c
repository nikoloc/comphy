#include "workspace.h"

#include <assert.h>
#include <stdlib.h>

#include "util/memory.h"

struct workspace *
workspace_create(struct state *state, struct output *output, int idx) {
    struct workspace *workspace = ALLOCATE(struct workspace);

    wl_list_init(&workspace->floats);
    wl_list_init(&workspace->slaves);

    workspace->output = output;
    workspace->idx = idx;
    string_init(&workspace->original_output_name, output->wlr_output->name);

    wl_list_insert(&output->workspaces, &workspace->link);

    if(!output->active_workspace) {
        output->active_workspace = workspace;
    }

    if(!state->active_workspace) {
        state->active_workspace = workspace;
    }

    return workspace;
}

void
workspace_destroy(struct state *state, struct workspace *workspace) {
    if(workspace->output->active_workspace == workspace) {
        // TODO: find new workspace to set as active
    }

    if(state->active_workspace == workspace) {
        // TODO: find new workspace to set as active
    }

    // TODO: evacuate toplevels and check if layers need some work

    string_deinit(&workspace->original_output_name);

    wl_list_remove(&workspace->link);
    free(workspace);
}

void
workspace_show_toplevels(struct workspace *workspace, bool show) {
    if(workspace->fullscreen) {
        wlr_scene_node_set_enabled(&workspace->fullscreen->scene_tree->node, show);
    }

    if(workspace->master) {
        wlr_scene_node_set_enabled(&workspace->master->scene_tree->node, show);
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->output->active_workspace->floats, link) {
        wlr_scene_node_set_enabled(&iter->scene_tree->node, show);
    }

    wl_list_for_each(iter, &workspace->output->active_workspace->slaves, link) {
        wlr_scene_node_set_enabled(&iter->scene_tree->node, show);
    }
}

void
workspace_set_active(struct state *state, struct workspace *workspace) {
    if(state->active_workspace == workspace) {
        // do nothing
        return;
    }

    // if it is an already active on its output, just switch to it
    if(workspace == workspace->output->active_workspace) {
        state->active_workspace = workspace;
        output_focus(state, workspace->output);
        return;
    }

    // else remove all the toplevels on that workspace
    struct workspace *old_workspace = workspace->output->active_workspace;
    workspace_show_toplevels(old_workspace, false);
    // and show this workspace's toplevels
    workspace_show_toplevels(workspace, true);

    if(state->active_workspace->output != workspace->output) {
        // if we are changing the output then warp the cursor
        // TODO: change so we only do this if there is no toplevel, else warp to toplevels
        output_warp_cursor(state, workspace->output);
    }

    state->active_workspace = workspace;
    workspace->output->active_workspace = workspace;

    output_focus(state, workspace->output);
}

struct workspace *
workspace_find_by_idx(struct state *state, int idx) {
    struct output *output;
    wl_list_for_each(output, &state->outputs, link) {
        struct workspace *workspace;
        wl_list_for_each(workspace, &output->workspaces, link) {
            if(workspace->idx == idx) {
                return workspace;
            }
        }
    }

    return NULL;
}

// TODO: move to `toplevel.c`
// void
// toplevel_move_to_workspace(struct mwc_toplevel *toplevel,
//                            struct mwc_workspace *workspace) {
//   assert(toplevel != NULL && workspace != NULL);
//   if(toplevel == server.grabbed_toplevel || toplevel->workspace == workspace
//      || workspace->fullscreen_toplevel != NULL) return;
//
//   struct mwc_workspace *old_workspace = toplevel->workspace;
//
//   /* handle server state; note: even tho fullscreen toplevel is handled differently
//    * we will still update its underlying type */
//   if(toplevel->floating) {
//     toplevel->workspace = workspace;
//     wl_list_remove(&toplevel->link);
//     wl_list_insert(&workspace->floating_toplevels, &toplevel->link);
//   } else if(toplevel_is_master(toplevel)){
//     wl_list_remove(&toplevel->link);
//     if(!wl_list_empty(&old_workspace->slaves)) {
//       struct mwc_toplevel *s = wl_container_of(old_workspace->slaves.next, s, link);
//       wl_list_remove(&s->link);
//       wl_list_insert(old_workspace->masters.prev, &s->link);
//     }
//
//     toplevel->workspace = workspace;
//     if(wl_list_length(&workspace->masters) < server.config->master_count) {
//       wl_list_insert(workspace->masters.prev, &toplevel->link);
//     } else {
//       wl_list_insert(workspace->slaves.prev, &toplevel->link);
//     }
//   } else {
//     wl_list_remove(&toplevel->link);
//
//     toplevel->workspace = workspace;
//     if(wl_list_length(&workspace->masters) < server.config->master_count) {
//       wl_list_insert(workspace->masters.prev, &toplevel->link);
//     } else {
//       wl_list_insert(workspace->slaves.prev, &toplevel->link);
//     }
//   }
//
//   /* handle presentation */
//   if(toplevel->fullscreen) {
//     old_workspace->fullscreen_toplevel = NULL;
//     workspace->fullscreen_toplevel = toplevel;
//
//     struct wlr_box output_box;
//     wlr_output_layout_get_box(server.output_layout, workspace->output->wlr_output, &output_box);
//     toplevel_set_pending_state(toplevel, output_box.x, output_box.y,
//                                output_box.width, output_box.height);
//
//     layers_under_fullscreen_set_enabled(workspace->output, false);
//     if(old_workspace->output != workspace->output) {
//       layers_under_fullscreen_set_enabled(old_workspace->output, true);
//     }
//
//     if(toplevel->floating) {
//       /* calculate where the toplevel should be placed after exiting fullscreen,
//        * see note for floating bellow */
//       uint32_t old_output_relative_x =
//         toplevel->prev_geometry.x - old_workspace->output->usable_area.x;
//       double relative_x =
//         (double)old_output_relative_x / old_workspace->output->usable_area.width;
//
//       uint32_t old_output_relative_y =
//         toplevel->prev_geometry.y - old_workspace->output->usable_area.y;
//       double relative_y =
//         (double)old_output_relative_y / old_workspace->output->usable_area.height;
//
//       uint32_t new_output_x = workspace->output->usable_area.x
//         + relative_x * workspace->output->usable_area.width;
//       uint32_t new_output_y = workspace->output->usable_area.y
//         + relative_y * workspace->output->usable_area.height;
//
//       toplevel->prev_geometry.x = new_output_x;
//       toplevel->prev_geometry.y = new_output_y;
//     } else {
//       layout_set_pending_state(old_workspace);
//     }
//   } else if(toplevel->floating && old_workspace->output != workspace->output) {
//     /* we want to place the toplevel to the same relative coordinates,
//      * as the new output may have a different resolution */
//     uint32_t old_output_relative_x =
//       toplevel->scene_tree->node.x - old_workspace->output->usable_area.x;
//     double relative_x =
//       (double)old_output_relative_x / old_workspace->output->usable_area.width;
//
//     uint32_t old_output_relative_y =
//       toplevel->scene_tree->node.y - old_workspace->output->usable_area.y;
//     double relative_y =
//       (double)old_output_relative_y / old_workspace->output->usable_area.height;
//
//     uint32_t new_output_x = workspace->output->usable_area.x
//       + relative_x * workspace->output->usable_area.width;
//     uint32_t new_output_y = workspace->output->usable_area.y
//       + relative_y * workspace->output->usable_area.height;
//
//     toplevel_set_pending_state(toplevel, new_output_x, new_output_y,
//                                toplevel->current.width, toplevel->current.height);
//   } else {
//     layout_set_pending_state(old_workspace);
//     layout_set_pending_state(workspace);
//   }
//
//   /* change active workspace */
//   change_workspace(workspace, true);
// }
//
