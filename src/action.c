#include "action.h"

#include <string.h>

#include "box_helpers.h"
#include "keybind.h"
#include "keyboard.h"
#include "layout.h"
#include "list_helpers.h"
#include "pointer.h"
#include "seat.h"
#include "system.h"
#include "util/macros.h"
#include "util/memory.h"
#include "wlr/util/log.h"
#include "workspace.h"

static inline color_t
get_color(struct state *state, struct toplevel *toplevel) {
    return toplevel == state->focused_toplevel ? state->config.border.color.active : state->config.border.color.active;
}

static void
update_all_borders(struct state *state) {
    struct output *output;
    wl_list_for_each(output, &state->outputs, link) {
        struct workspace *workspace;
        wl_list_for_each(workspace, &output->workspaces, link) {
            if(workspace->master) {
                color_t color = get_color(state, workspace->master);
                toplevel_set_border_color(workspace->master, color);
            }

            struct toplevel *toplevel;
            wl_list_for_each(toplevel, &workspace->floats, link) {
                color_t color = get_color(state, toplevel);
                toplevel_set_border_color(toplevel, color);
            }

            wl_list_for_each(toplevel, &workspace->slaves, link) {
                color_t color = get_color(state, toplevel);
                toplevel_set_border_color(toplevel, color);
            }
        }
    }
}

static struct output *
get_cross_output(struct state *state, struct toplevel *toplevel, enum wlr_direction direction) {
    int x, y;
    wlr_box_midpoint(&toplevel->current, &x, &y);

    struct wlr_output *wlr_output = wlr_output_layout_adjacent_output(state->output_layout, direction,
            toplevel->workspace->output->wlr_output, x, y);
    if(!wlr_output) {
        return NULL;
    }

    return wlr_output->data;
}

static void
focus_cross_output(struct state *state, struct toplevel *toplevel, enum wlr_direction direction) {
    struct output *output = get_cross_output(state, toplevel, direction);
    if(output) {
        output_focus(state, output);
    }
}

static struct toplevel *
find_in_direction(struct toplevel *toplevel, enum wlr_direction direction) {
    ASSERT(toplevel->state == TOPLEVEL_STATE_TILED);

    struct workspace *workspace = toplevel->workspace;
    switch(direction) {
        case WLR_DIRECTION_UP: {
            if(toplevel != workspace->master) {
                // slave, try finding other slave above
                struct wl_list *above = wl_list_prev(&workspace->slaves, &toplevel->link);
                if(above) {
                    return CONTAINER_OF(above, struct toplevel, link);
                }
            }

            // if master then there is none
            break;
        }
        case WLR_DIRECTION_DOWN: {
            // same as above
            if(toplevel != workspace->master) {
                struct wl_list *bellow = wl_list_next(&workspace->slaves, &toplevel->link);
                if(bellow) {
                    return CONTAINER_OF(bellow, struct toplevel, link);
                }
            }

            break;
        }
        case WLR_DIRECTION_LEFT: {
            // if slave then master, if master then none
            if(toplevel != workspace->master) {
                return workspace->master;
            }

            break;
        }
        case WLR_DIRECTION_RIGHT: {
            // if master then bottom slave if such, if slave then none
            if(toplevel == workspace->master) {
                struct wl_list *bottom = wl_list_last(&workspace->slaves);
                if(bottom) {
                    return CONTAINER_OF(bottom, struct toplevel, link);
                }
            }

            break;
        }
    }

    return NULL;
}

static void
focus(struct state *state, enum wlr_direction direction) {
    struct toplevel *toplevel = state->focused_toplevel;
    if(!toplevel || toplevel == state->grabbed_toplevel) {
        return;
    }

    if(toplevel->state == TOPLEVEL_STATE_TILED) {
        // for tiled try to figure out the next one from the layout
        struct toplevel *focus_next = find_in_direction(toplevel, direction);
        if(focus_next) {
            toplevel_focus(state, focus_next);
            return;
        }
    }

    // for floats and fullscreened toplevels we just move focus across outputs; doing anything else just does not
    // seem worth the work tbh as any other way of doing would not feed any more natural and would need more work
    focus_cross_output(state, toplevel, direction);
}

static void
move_cross_output(struct state *state, struct toplevel *toplevel, enum wlr_direction direction) {
    struct output *output = get_cross_output(state, toplevel, direction);
    if(output) {
        toplevel_move_to_workspace(state, toplevel, output->active_workspace);
    }
}

static void
swap_layout(struct toplevel *a, struct toplevel *b, enum wlr_direction direction) {
    ASSERT(a->workspace == b->workspace);

    struct workspace *workspace = a->workspace;
    if(a == workspace->master) {
        // save the link before b and remove b
        struct wl_list *prev = b->link.prev;
        wl_list_remove(&b->link);

        workspace->master = b;
        wl_list_insert(prev, &a->link);
    } else if(b == workspace->master) {
        // same but the other way around
        struct wl_list *prev = a->link.prev;
        wl_list_remove(&a->link);

        workspace->master = a;
        wl_list_insert(prev, &b->link);
    } else {
        // here we need to know which one is before the other in the list. figure out based on the direction
        if(direction == WLR_DIRECTION_UP) {
            // a is after b. save prev
            struct wl_list *prev = b->link.prev;
            // remove b, add b, remove a, add a
            wl_list_remove(&b->link);
            wl_list_insert(&a->link, &b->link);
            wl_list_remove(&a->link);
            wl_list_insert(prev, &a->link);
        } else {
            struct wl_list *prev = a->link.prev;

            wl_list_remove(&a->link);
            wl_list_insert(&b->link, &a->link);
            wl_list_remove(&b->link);
            wl_list_insert(prev, &b->link);
        }
    }
}

static void
move(struct state *state, enum wlr_direction direction) {
    // similar as above, fallback to outputs if the layout change fails
    struct toplevel *toplevel = state->focused_toplevel;
    if(!toplevel || toplevel == state->grabbed_toplevel) {
        return;
    }

    if(toplevel->state == TOPLEVEL_STATE_TILED) {
        // for tiled try to figure out the next one from the layout
        struct toplevel *swap = find_in_direction(toplevel, direction);
        if(swap) {
            swap_layout(toplevel, swap, direction);
            layout_configure(state, toplevel->workspace);
            return;
        }
    }

    move_cross_output(state, toplevel, direction);
}

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
            struct action_focus *action = _action;
            wlr_log(WLR_ERROR, "here");
            focus(state, action->direction);
            break;
        }
        case ACTION_TYPE_MOVE: {
            struct action_move *action = _action;
            move(state, action->direction);
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

            update_all_borders(state);
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
