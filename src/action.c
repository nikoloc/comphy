#include "action.h"

#include "keyboard.h"
#include "pointer.h"
#include "state.h"
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
            struct action_focus *action = _action;
            // TODO: this
            break;
        }
        case ACTION_TYPE_MOVE: {
            break;
        }
        case ACTION_TYPE_EXEC: {
            break;
        }
        case ACTION_TYPE_ENV: {
            struct action_env *action = _action;

            setenv(action->key, action->value, true);
            break;
        }
        case ACTION_TYPE_KEYBOARD: {
            struct action_keyboard *action = _action;

            if(action->rate) {
                state->config.keyboard_rate = action->rate;
            }

            if(action->delay) {
                state->config.keyboard_delay = action->delay;
            }

            // go through all the keyboards and apply new state
            struct keyboard *iter;
            wl_list_for_each(iter, &state->keyboards, link) {
                wlr_keyboard_set_repeat_info(iter->wlr_keyboard, state->config.keyboard_rate,
                        state->config.keyboard_delay);
                // TODO: xkb stuff or maybe wrap everyhing into a structure and keyboard_configure()
            }
            break;
        }
        case ACTION_TYPE_POINTER: {
            break;
        }
        case ACTION_TYPE_TRACKPAD: {
            break;
        }
        case ACTION_TYPE_CURSOR: {
            break;
        }
        case ACTION_TYPE_GAPS: {
            break;
        }
        case ACTION_TYPE_BORDER: {
            break;
        }
        case ACTION_TYPE_TOPLEVEL: {
            break;
        }
        case ACTION_TYPE_ADJUST_MASTER_RATIO: {
            break;
        }
        case ACTION_TYPE_SET_MASTER_RATIO: {
            break;
        }
        case ACTION_TYPE_CREATE_KEYBINDS: {
            break;
        }
    }
}
