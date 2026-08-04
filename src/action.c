#include "action.h"

#include <string.h>

#include "keybind.h"
#include "keyboard.h"
#include "layout.h"
#include "pointer.h"
#include "seat.h"
#include "system.h"
#include "util/memory.h"
#include "wlr/util/log.h"
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

            wlr_log(WLR_ERROR, "start");
            struct workspace *workspace = workspace_find_by_idx(state, action->idx);
            if(workspace) {
                wlr_log(WLR_ERROR, "found");
                workspace_set_active(state, workspace);
            } else {
                wlr_log(WLR_ERROR, "not found");
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
        case ACTION_TYPE_CLOSE: {
            struct toplevel *toplevel = state->focused_toplevel;
            if(!toplevel) {
                break;
            }

            wlr_xdg_toplevel_send_close(toplevel->wlr_toplevel);
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

                    layout_configure(state, workspace);
                    toplevel_configure(state, toplevel, &(struct wlr_box){0});
                    toplevel->needs_centering = true;
                    break;
                }
                case TOPLEVEL_STATE_FLOAT: {
                    struct workspace *workspace = toplevel->workspace;
                    wl_list_remove(&toplevel->link);
                    layout_add(workspace, toplevel);

                    layout_configure(state, workspace);
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
            enum view *pointer_focused = seat_get_pointer_focused(state);
            if(!pointer_focused) {
                break;
            }

            struct toplevel *toplevel = view_get_toplevel(pointer_focused);
            if(!toplevel) {
                break;
            }

            operation_start_move(state, toplevel);
            break;
        }
        case ACTION_TYPE_START_RESIZE: {
            enum view *pointer_focused = seat_get_pointer_focused(state);
            if(!pointer_focused) {
                break;
            }

            struct toplevel *toplevel = view_get_toplevel(pointer_focused);
            if(!toplevel) {
                break;
            }

            u32 edges =
                    toplevel_get_corner_closest_to(toplevel, state->cursor.wlr_cursor->x, state->cursor.wlr_cursor->y);

            operation_start_resize(state, toplevel, edges);
            break;
        }
        case ACTION_TYPE_REPEAT_RATE: {
            struct action_repeat_rate *action = _action;

            state->config.keyboard.rate = action->rate;
            state->config.keyboard.delay = action->delay;

            struct keyboard *iter;
            wl_list_for_each(iter, &state->keyboards, link) {
                keyboard_set_repeat_rate(iter, action->rate, action->delay);
            }

            break;
        }
        case ACTION_TYPE_KEYMAP: {
            struct action_keymap *action = _action;

            FREE(state->config.keyboard.xkb_layouts);
            FREE(state->config.keyboard.xkb_variants);
            FREE(state->config.keyboard.xkb_options);
            state->config.keyboard.xkb_layouts = strdup(action->xkb_layouts);
            state->config.keyboard.xkb_variants = strdup(action->xkb_variants);
            state->config.keyboard.xkb_options = strdup(action->xkb_options);

            struct keyboard *iter;
            wl_list_for_each(iter, &state->keyboards, link) {
                keyboard_set_keymap(iter, action->xkb_layouts, action->xkb_variants, action->xkb_options);
            }
            break;
        }
        case ACTION_TYPE_TRACKPAD_DISABLE_WHILE_TYPING: {
            struct action_trackpad_dwt *action = _action;

            state->config.trackpad.dwt = action->enable;

            struct pointer *iter;
            wl_list_for_each(iter, &state->pointers, link) {
                pointer_if_trackpad_set_dwt(iter, action->enable);
            }
            break;
        }
        case ACTION_TYPE_TRACKPAD_TAP_TO_CLICK: {
            struct action_trackpad_tap_to_click *action = _action;

            state->config.trackpad.tap_to_click = action->enable;

            struct pointer *iter;
            wl_list_for_each(iter, &state->pointers, link) {
                pointer_if_trackpad_set_tap_to_click(iter, action->enable);
            }
            break;
        }
        case ACTION_TYPE_TRACKPAD_NATURAL_SCROLL: {
            struct action_trackpad_natural_scroll *action = _action;

            state->config.trackpad.natural_scroll = action->enable;

            struct pointer *iter;
            wl_list_for_each(iter, &state->pointers, link) {
                pointer_if_trackpad_set_natural_scroll(iter, action->enable);
            }
            break;
        }
        case ACTION_TYPE_TRACKPAD_SCROLL_METHOD: {
            struct action_trackpad_scroll_method *action = _action;

            state->config.trackpad.scroll_method = action->value;

            struct pointer *iter;
            wl_list_for_each(iter, &state->pointers, link) {
                pointer_if_trackpad_set_scroll_method(iter, action->value);
            }
            break;
        }
        case ACTION_TYPE_CURSOR_THEME: {
            struct action_cursor_theme *action = _action;
            cursor_set_theme(&state->cursor, action->theme, action->size);
            break;
        }
        case ACTION_TYPE_CURSOR_WARP: {
            struct action_cursor_warp *action = _action;
            state->config.cursor.warp = action->value;
            break;
        }
        case ACTION_TYPE_CURSOR_HIDE_AFTER_MS: {
            struct action_cursor_hide_after_ms *action = _action;
            state->config.cursor.hide_after_ms = action->value;
            break;
        }
        case ACTION_TYPE_GAPS: {
            struct action_gaps *action = _action;
            state->config.gaps.outer = action->outer;
            state->config.gaps.inner = action->inner;

            layout_reconfigure_all(state);
            break;
        }
        case ACTION_TYPE_SMART_GAPS: {
            struct action_smart_gaps *action = _action;
            state->config.gaps.smart = action->enable;

            layout_reconfigure_all(state);
            break;
        }
        case ACTION_TYPE_BORDER_WIDTH: {
            struct action_border_width *action = _action;
            state->config.border.width = action->value;

            layout_reconfigure_all(state);
            break;
        }
        case ACTION_TYPE_BORDER_COLOR: {
            struct action_border_color *action = _action;
            state->config.border.color.active = action->active;
            state->config.border.color.inactive = action->inactive;
            // TODO: update for all clients
            break;
        }
        case ACTION_TYPE_MASTER_RATIO: {
            struct action_master_ratio *action = _action;
            if(action->adjust) {
                // TODO: move this to per workspace option
                state->config.master_ratio += action->value;
            } else {
                state->config.master_ratio = action->value;
            }

            layout_reconfigure_all(state);
            break;
        }
        case ACTION_TYPE_TOPLEVEL_RULE: {
            break;
        }
        case ACTION_TYPE_POINTER_RULE: {
            break;
        }
        case ACTION_TYPE_CREATE_KEYBIND: {
            struct keybind *keybind = _action;

            wl_list_insert(&state->keybinds, &keybind->link);
            break;
        }
    }
}
