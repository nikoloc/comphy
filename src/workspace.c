#include "workspace.h"

#include <assert.h>
#include <wlr/types/wlr_ext_workspace_v1.h>
#include <wlr/util/log.h>

#include "list_helpers.h"
#include "util/macros.h"
#include "util/memory.h"

struct workspace *
workspace_create(struct state *state, struct output *output, int idx) {
    if(output->dummy_workspace) {
        // this is the first real workspace created for this output, so we dont really create it, but just rename and
        // remove the dummy workspace instead
        struct workspace *workspace = output->dummy_workspace;
        output->dummy_workspace = NULL;

        workspace->idx = idx;

        char buffer[8] = {0};
        snprintf(buffer, sizeof(buffer), "%d", workspace->idx);
        wlr_ext_workspace_handle_v1_set_name(workspace->ext_workspace, buffer);

        wlr_log(WLR_INFO, "replaced dummy workspace of output '%s' to workspace '%d'", output->wlr_output->name, idx);

        return workspace;
    }

    struct workspace *workspace = ALLOC(struct workspace);

    wl_list_init(&workspace->floats);
    wl_list_init(&workspace->slaves);

    wl_list_init(&workspace->ghosts);

    workspace->output = output;
    workspace->idx = idx;
    workspace->original_output_name = strdup(output->wlr_output->name);

    // init ext workspace handle
    char buffer[8] = {0};
    snprintf(buffer, sizeof(buffer), "%d", workspace->idx);
    workspace->ext_workspace = wlr_ext_workspace_handle_v1_create(state->ext_workspace_mgr.wlr_mgr, buffer,
            EXT_WORKSPACE_HANDLE_V1_WORKSPACE_CAPABILITIES_ACTIVATE |
                    EXT_WORKSPACE_HANDLE_V1_WORKSPACE_CAPABILITIES_REMOVE);
    wlr_ext_workspace_handle_v1_set_name(workspace->ext_workspace, buffer);
    wlr_ext_workspace_handle_v1_set_group(workspace->ext_workspace, output->ext_workspace_group);
    workspace->ext_workspace->data = workspace;

    wl_list_insert(&output->workspaces, &workspace->link);

    if(!output->active_workspace) {
        output->active_workspace = workspace;
    }

    if(!state->active_workspace) {
        state->active_workspace = workspace;
        wlr_ext_workspace_handle_v1_set_active(workspace->ext_workspace, true);
    }

    wlr_log(WLR_INFO, "created workspace for output '%s' indexed '%d'", output->wlr_output->name, idx);

    return workspace;
}

static struct workspace *
find_next_on_output(struct state *state, struct workspace *workspace) {
    UNUSED(state);

    struct output *output = workspace->output;
    struct wl_list *next = wl_list_next_or_prev(&output->workspaces, &workspace->link);
    if(!next) {
        // create a new dummy
        wlr_log(WLR_INFO, "output '%s' left with no workspaces, creating dummy", workspace->output->wlr_output->name);
        output->dummy_workspace = workspace_create(state, output, -1);
        return output->dummy_workspace;
    }

    return CONTAINER_OF(next, struct workspace, link);
}

static void
evacuate(struct state *state, struct workspace *workspace, struct workspace *next) {
    // evacuate toplevels
    if(workspace->fullscreen) {
        toplevel_move_to_workspace(state, workspace->fullscreen, next);
    }

    struct toplevel *iter, *tmp;
    wl_list_for_each_safe(iter, tmp, &workspace->floats, link) {
        toplevel_move_to_workspace(state, iter, next);
    }

    // since moving master out of the workspace promotes new slave to a master we just move master until there are none
    while(workspace->master) {
        toplevel_move_to_workspace(state, workspace->master, next);
    }
}

void
workspace_destroy(struct state *state, struct workspace *workspace, bool output_is_destroying) {
    struct output *output = workspace->output;

    struct workspace *next = NULL;
    if(!output_is_destroying) {
        next = find_next_on_output(state, workspace);

        if(workspace == output->active_workspace) {
            output->active_workspace = next;
        }
    } else {
        struct wl_list *next_output_link = wl_list_next_or_prev(&state->outputs, &output->link);
        if(next_output_link) {
            struct output *output = CONTAINER_OF(next_output_link, struct output, link);
            next = CONTAINER_OF(output->workspaces.next, struct workspace, link);
        }
    }

    if(next) {
        wlr_log(WLR_DEBUG, "found next workspace to evacuate and focus '%d'", next->idx);

        evacuate(state, workspace, next);
        if(workspace == state->active_workspace) {
            // also set it as global active
            workspace_set_active(state, next, false);
        }
    } else {
        wlr_log(WLR_INFO, "no outputs left");
        // TODO: handle no outputs
    }

    wl_list_remove(&workspace->link);

    wlr_ext_workspace_handle_v1_destroy(workspace->ext_workspace);

    if(workspace == output->presented_workspace) {
        output->presented_workspace = NULL;
    }

    if(workspace->transaction_time_out) {
        wl_event_source_remove(workspace->transaction_time_out);
    }

    if(workspace->transaction_schedule) {
        wl_event_source_remove(workspace->transaction_schedule);
    }

    FREE(workspace->original_output_name);
    FREE(workspace);
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
    wl_list_for_each(iter, &workspace->slaves, link) {
        wlr_scene_node_set_enabled(&iter->scene_tree->node, show);
    }

    wl_list_for_each(iter, &workspace->floats, link) {
        wlr_scene_node_set_enabled(&iter->scene_tree->node, show);
    }

    // also ghosts
    wl_list_for_each(iter, &workspace->ghosts, link) {
        wlr_scene_node_set_enabled(&iter->scene_tree->node, show);
    }
}

void
workspace_set_active(struct state *state, struct workspace *workspace, bool keep_focus) {
    if(state->active_workspace == workspace) {
        // do nothing
        return;
    }

    struct workspace *old_workspace = state->active_workspace;

    state->active_workspace = workspace;
    workspace->output->active_workspace = workspace;

    if(!keep_focus) {
        output_focus(state, workspace->output);
    }

    wlr_ext_workspace_handle_v1_set_active(old_workspace->ext_workspace, false);
    wlr_ext_workspace_handle_v1_set_active(workspace->ext_workspace, true);

    // commit on the transaction, tho we need to schedule one is the workspace is clean
    transaction_schedule_commit(state, workspace);
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

bool
workspace_is_presented(struct workspace *workspace) {
    struct output *output = workspace->output;
    if(!output) {
        return false;
    }

    return output->presented_workspace == workspace;
}
