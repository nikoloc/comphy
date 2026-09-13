#include "ext_workspace.h"

#include <wlr/util/log.h>

#include "comphy.h"
#include "state.h"
#include "util/macros.h"
#include "util/parse.h"
#include "workspace.h"

static void
handle_request(struct state *state, struct wlr_ext_workspace_v1_request *request) {
    switch(request->type) {
        case WLR_EXT_WORKSPACE_V1_REQUEST_CREATE_WORKSPACE: {
            if(!request->create_workspace.group) {
                break;
            }

            struct output *output = request->create_workspace.group->data;
            ASSERT(output);

            int idx;
            if(!parse_int(request->create_workspace.name, &idx)) {
                wlr_log(WLR_ERROR, "tried to create a workspace with no integer name");
                break;
            }

            workspace_create(state, output, idx);
            break;
        }
        case WLR_EXT_WORKSPACE_V1_REQUEST_ACTIVATE: {
            if(!request->activate.workspace) {
                break;
            }

            struct workspace *workspace = request->activate.workspace->data;
            ASSERT(workspace);

            workspace_set_active(state, workspace, false);
            break;
        }
        case WLR_EXT_WORKSPACE_V1_REQUEST_DEACTIVATE: {
            // noop
            break;
        }
        case WLR_EXT_WORKSPACE_V1_REQUEST_ASSIGN: {
            // when reparent workspace api
            break;
        }
        case WLR_EXT_WORKSPACE_V1_REQUEST_REMOVE: {
            if(!request->remove.workspace) {
                break;
            }

            struct workspace *workspace = request->remove.workspace->data;
            ASSERT(workspace);

            workspace_destroy(state, workspace, false);
            break;
        }
    }
}

static void
handle_commit(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_ext_workspace_v1_commit_event *event = data;
    struct wlr_ext_workspace_v1_request *request;
    struct state *state = state_get();

    wl_list_for_each(request, event->requests, link) {
        handle_request(state, request);
    }
}

void
ext_workspace_mgr_init(struct ext_workspace_mgr *mgr, struct wl_display *display) {
    mgr->wlr_mgr = wlr_ext_workspace_manager_v1_create(display, COMPHY_WORKSPACE_VERSION);

    mgr->commit.notify = handle_commit;
    wl_signal_add(&mgr->wlr_mgr->events.commit, &mgr->commit);
}

void
ext_workspace_mgr_deinit(struct ext_workspace_mgr *mgr) {
    wl_list_remove(&mgr->commit.link);
}
