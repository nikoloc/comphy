#include "action.h"

#include "layout.h"
#include "system.h"
#include "workspace.h"

void
action_perform(struct state *state, enum action_type type, void *_action) {
    switch(type) {
        case ACTION_TYPE_CREATE_WORKSPACE: {
            struct action_create_workspace *action = _action;

            struct output *output;
            if(!action->output || !(output = output_find_by_name(state, action->output))) {
                // else take the current output
                output = state->active_workspace->output;
            }

            workspace_create(state, output, action->idx);
            break;
        }
        case ACTION_TYPE_CHANGE_WORKSPACE: {
            struct action_change_workspace *action = _action;

            struct workspace *workspace = workspace_find_by_idx(state, action->idx);
            if(workspace) {
                workspace_set_active(state, workspace);
            }
            break;
        }
        case ACTION_TYPE_MOVE_TO_WORKSPACE: {
            struct action_move_to_workspace *action = _action;
            if(!state->focused_toplevel) {
                return;
            }

            struct workspace *workspace = workspace_find_by_idx(state, action->idx);
            if(workspace) {
                toplevel_move_to_workspace(state, state->focused_toplevel, workspace);
            }
            break;
        }
        case ACTION_TYPE_FOCUS: {
            // TODO
            break;
        }
        case ACTION_TYPE_MOVE: {
            // TODO
            break;
        }
        case ACTION_TYPE_EXEC: {
            struct action_exec *action = _action;
            shell(action->cmd);
            break;
        }
        case ACTION_TYPE_ENV: {
            struct action_env *action = _action;

            setenv(action->key, action->value, true);
            break;
        }
        case ACTION_TYPE_EXIT: {
            wl_display_terminate(state->display);
            // TODO: remove this flag by being smarter
            state->is_exiting = true;
            break;
        }
        case ACTION_TYPE_TOGGLE_FLOAT: {
            struct toplevel *toplevel = state->focused_toplevel;
            if(!toplevel) {
                break;
            }

            switch(toplevel->state) {
                case TOPLEVEL_STATE_TILED: {
                    struct workspace *workspace = toplevel->workspace;
                    layout_remove(toplevel);
                    wl_list_insert(&workspace->floats, &toplevel->link);
                    break;
                }
                case TOPLEVEL_STATE_FLOAT: {
                    struct workspace *workspace = toplevel->workspace;
                    wl_list_remove(&toplevel->link);
                    layout_add(workspace, toplevel);
                    break;
                }
                case TOPLEVEL_STATE_FULLSCREEN: {
                    // no op
                    break;
                }
            }

            break;
        }
        case ACTION_TYPE_TOGGLE_FULLSCREEN: {
            struct toplevel *toplevel = state->focused_toplevel;
            if(!toplevel) {
                break;
            }

            toplevel_set_fullscreen(state, toplevel, toplevel->state != TOPLEVEL_STATE_FULLSCREEN);
            break;
        }
        case ACTION_TYPE_TOGGLE_FAKE_FULLSCREEN: {
            // TODO
            break;
        }
        case ACTION_TYPE_START_MOVE: {
            struct toplevel *toplevel = state->focused_toplevel;
            if(!toplevel) {
                break;
            }

            operation_start_move(state, toplevel);
            break;
        }
        case ACTION_TYPE_START_RESIZE: {
            break;
        }
        case ACTION_TYPE_REPEAT_RATE: {
            break;
        }
        case ACTION_TYPE_KEYMAP: {
            break;
        }
        case ACTION_TYPE_TRACKPAD_DISABLE_WHILE_TYPING: {
            break;
        }
        case ACTION_TYPE_TRACKPAD_NATURAL_SCROLL: {
            break;
        }
        case ACTION_TYPE_TRACKPAD_SCROLL_METHOD: {
            break;
        }
        case ACTION_TYPE_CURSOR_THEME: {
            break;
        }
        case ACTION_TYPE_CURSOR_WARP: {
            break;
        }
        case ACTION_TYPE_CURSOR_HIDE_AFTER_MS: {
            break;
        }
        case ACTION_TYPE_GAPS: {
            break;
        }
        case ACTION_TYPE_SMART_GAPS: {
            break;
        }
        case ACTION_TYPE_BORDER_WIDTH: {
            break;
        }
        case ACTION_TYPE_BORDER_COLOR: {
            break;
        }
        case ACTION_TYPE_MASTER_RATIO: {
            break;
        }
        case ACTION_TYPE_TOPLEVEL_RULE: {
            break;
        }
        case ACTION_TYPE_POINTER_RULE: {
            break;
        }
        case ACTION_TYPE_CREATE_KEYBIND: {
            break;
        }
    }
}
