#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "action.h"
#include "keybind.h"
#include "util/memory.h"
#include "util/parse.h"
#define SHELL_PARSER_IMPLEMENTATION
#include "util/shell_parser.h"

// this file, alongside `action_types.h` was largely ai generated and patched afterwards from the specification found in
// the `comphyctl_commands.txt` file. if the file is regenerated this notice should be preserved.

void
action_destroy(enum action_type type, void *_action) {
    if(!_action) {
        // may happen with non fully inited keybinds
        return;
    }

    switch(type) {
        case ACTION_TYPE_CREATE_WORKSPACE: {
            struct action_create_workspace *action = _action;
            FREE(action->output);
            break;
        }
        case ACTION_TYPE_CHANGE_WORKSPACE: {
            break;
        }
        case ACTION_TYPE_MOVE_TO_WORKSPACE: {
            break;
        }
        case ACTION_TYPE_FOCUS: {
            break;
        }
        case ACTION_TYPE_MOVE: {
            break;
        }
        case ACTION_TYPE_EXEC: {
            struct action_exec *action = _action;
            FREE(action->cmd);
            break;
        }
        case ACTION_TYPE_ENV: {
            struct action_env *action = _action;
            FREE(action->key);
            FREE(action->value);
            break;
        }
        case ACTION_TYPE_CLOSE: {
            break;
        }
        case ACTION_TYPE_EXIT: {
            break;
        }
        case ACTION_TYPE_TOGGLE_FLOAT: {
            break;
        }
        case ACTION_TYPE_TOGGLE_FULLSCREEN: {
            break;
        }
        case ACTION_TYPE_TOGGLE_FAKE_FULLSCREEN: {
            break;
        }
        case ACTION_TYPE_START_MOVE: {
            break;
        }
        case ACTION_TYPE_START_RESIZE: {
            break;
        }
        case ACTION_TYPE_REPEAT_RATE: {
            break;
        }
        case ACTION_TYPE_KEYMAP: {
            struct action_keymap *action = _action;
            FREE(action->xkb_layouts);
            FREE(action->xkb_variants);
            FREE(action->xkb_options);
            break;
        }
        case ACTION_TYPE_TRACKPAD_DISABLE_WHILE_TYPING: {
            break;
        }
        case ACTION_TYPE_TRACKPAD_TAP_TO_CLICK: {
            break;
        }
        case ACTION_TYPE_TRACKPAD_NATURAL_SCROLL: {
            break;
        }
        case ACTION_TYPE_TRACKPAD_SCROLL_METHOD: {
            break;
        }
        case ACTION_TYPE_CURSOR_THEME: {
            struct action_cursor_theme *action = _action;
            FREE(action->theme);
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
        case ACTION_TYPE_CLIENT_SIDE_DECORATIONS: {
            break;
        }
        case ACTION_TYPE_TOPLEVEL_RULE: {
            // TODO: implement
            break;
        }
        case ACTION_TYPE_POINTER_RULE: {
            // TODO: implement
            break;
        }
        case ACTION_TYPE_CREATE_KEYBIND: {
            // in order to not free this action, since it needs to survive, we set the pointer to NULL here. not the
            // best solution, but is less to type
            _action = NULL;
            break;
        }
    }

    FREE(_action);
}

static bool
parse_cursor_warp(const char *str, enum cursor_warp *warp) {
    if(strcmp(str, "none") == 0) {
        *warp = CURSOR_WARP_NONE;
        return true;
    }
    if(strcmp(str, "on_focus_change") == 0) {
        *warp = CURSOR_WARP_ON_FOCUS_CHANGE;
        return true;
    }
    if(strcmp(str, "on_output_change") == 0) {
        *warp = CURSOR_WARP_ON_OUTPUT_CHANGE;
        return true;
    }
    return false;
}

static bool
parse_direction(const char *str, enum wlr_direction *dir) {
    if(strcmp(str, "up") == 0) {
        *dir = WLR_DIRECTION_UP;
        return true;
    }
    if(strcmp(str, "right") == 0) {
        *dir = WLR_DIRECTION_RIGHT;
        return true;
    }
    if(strcmp(str, "down") == 0) {
        *dir = WLR_DIRECTION_DOWN;
        return true;
    }
    if(strcmp(str, "left") == 0) {
        *dir = WLR_DIRECTION_LEFT;
        return true;
    }
    return false;
}

static bool
parse_bool(const char *str, bool *val) {
    if(strcmp(str, "on") == 0) {
        *val = true;
        return true;
    }
    if(strcmp(str, "off") == 0) {
        *val = false;
        return true;
    }
    return false;
}

static bool
parse_scroll_method(const char *str, enum libinput_config_scroll_method *method) {
    if(strcmp(str, "none") == 0) {
        *method = LIBINPUT_CONFIG_SCROLL_NO_SCROLL;
        return true;
    }
    if(strcmp(str, "two_fingers") == 0) {
        *method = LIBINPUT_CONFIG_SCROLL_2FG;
        return true;
    }
    if(strcmp(str, "edge") == 0) {
        *method = LIBINPUT_CONFIG_SCROLL_EDGE;
        return true;
    }
    if(strcmp(str, "on_button_down") == 0) {
        *method = LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN;
        return true;
    }
    return false;
}

static bool
parse_modifiers(const char *str, uint32_t *modifiers) {
    *modifiers = 0;
    if(!str || strlen(str) == 0) {
        return true;
    }

    char *copy = strdup(str);
    if(!copy) {
        return false;
    }

    char *token = strtok(copy, "+");
    while(token) {
        if(strcasecmp(token, "ctrl") == 0 || strcasecmp(token, "control") == 0) {
            *modifiers |= WLR_MODIFIER_CTRL;
        } else if(strcasecmp(token, "shift") == 0) {
            *modifiers |= WLR_MODIFIER_SHIFT;
        } else if(strcasecmp(token, "alt") == 0 || strcasecmp(token, "mod1") == 0) {
            *modifiers |= WLR_MODIFIER_ALT;
        } else if(strcasecmp(token, "super") == 0 || strcasecmp(token, "logo") == 0 || strcasecmp(token, "win") == 0 ||
                  strcasecmp(token, "mod4") == 0) {
            *modifiers |= WLR_MODIFIER_LOGO;
        }
        token = strtok(NULL, "+");
    }

    free(copy);
    return true;
}

static bool
parse_keysym(const char *str, uint32_t *keysym) {
    if(strcasecmp(str, "return") == 0 || strcasecmp(str, "enter") == 0) {
        *keysym = XKB_KEY_Return;
    } else if(strcasecmp(str, "backspace") == 0) {
        *keysym = XKB_KEY_BackSpace;
    } else if(strcasecmp(str, "delete") == 0) {
        *keysym = XKB_KEY_Delete;
    } else if(strcasecmp(str, "escape") == 0) {
        *keysym = XKB_KEY_Escape;
    } else if(strcasecmp(str, "tab") == 0) {
        *keysym = XKB_KEY_Tab;
    } else if(strcasecmp(str, "up") == 0) {
        *keysym = XKB_KEY_Up;
    } else if(strcasecmp(str, "down") == 0) {
        *keysym = XKB_KEY_Down;
    } else if(strcasecmp(str, "left") == 0) {
        *keysym = XKB_KEY_Left;
    } else if(strcasecmp(str, "right") == 0) {
        *keysym = XKB_KEY_Right;
    } else {
        *keysym = xkb_keysym_from_name(str, 0);
        if(*keysym == 0) {
            wlr_log(WLR_ERROR, "key '%s' doesn't seem right", str);
            return false;
        }
    }
    return true;
}

bool
action_create(struct shell_parser *parser, enum action_type *out_type, void **dest) {
    char word[1024] = {0};

    if(!shell_parser_pop(parser, sizeof(word), word)) {
        return false;
    }

    if(strcmp(word, "create_workspace") == 0) {
        *out_type = ACTION_TYPE_CREATE_WORKSPACE;
        struct action_create_workspace *action = ALLOC(struct action_create_workspace);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }

        if(strcmp(word, "--output") == 0) {
            if(!shell_parser_pop(parser, sizeof(word), word)) {
                action_destroy(*out_type, *dest);
                *dest = NULL;
                return false;
            }
            action->output = strdup(word);

            if(!shell_parser_pop(parser, sizeof(word), word)) {
                action_destroy(*out_type, *dest);
                *dest = NULL;
                return false;
            }
        }

        if(!parse_int(word, &action->idx)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "change_workspace") == 0) {
        *out_type = ACTION_TYPE_CHANGE_WORKSPACE;
        struct action_change_workspace *action = ALLOC(struct action_change_workspace);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->idx)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "move_to_workspace") == 0) {
        *out_type = ACTION_TYPE_MOVE_TO_WORKSPACE;
        struct action_move_to_workspace *action = ALLOC(struct action_move_to_workspace);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->idx)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "focus") == 0) {
        *out_type = ACTION_TYPE_FOCUS;
        struct action_focus *action = ALLOC(struct action_focus);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_direction(word, &action->direction)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "move") == 0) {
        *out_type = ACTION_TYPE_MOVE;
        struct action_move *action = ALLOC(struct action_move);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_direction(word, &action->direction)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "exec") == 0) {
        *out_type = ACTION_TYPE_EXEC;
        struct action_exec *action = ALLOC(struct action_exec);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        action->cmd = strdup(word);
        return true;

    } else if(strcmp(word, "env") == 0) {
        *out_type = ACTION_TYPE_ENV;
        struct action_env *action = ALLOC(struct action_env);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        action->key = strdup(word);

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        action->value = strdup(word);
        return true;

    } else if(strcmp(word, "exit") == 0) {
        *out_type = ACTION_TYPE_EXIT;
        *dest = NULL;
        return true;

    } else if(strcmp(word, "close") == 0) {
        *out_type = ACTION_TYPE_CLOSE;
        *dest = NULL;
        return true;

    } else if(strcmp(word, "toggle_float") == 0) {
        *out_type = ACTION_TYPE_TOGGLE_FLOAT;
        *dest = NULL;
        return true;

    } else if(strcmp(word, "toggle_fullscreen") == 0) {
        *out_type = ACTION_TYPE_TOGGLE_FULLSCREEN;
        *dest = NULL;
        return true;

    } else if(strcmp(word, "toggle_fake_fullscreen") == 0) {
        *out_type = ACTION_TYPE_TOGGLE_FAKE_FULLSCREEN;
        *dest = NULL;
        return true;

    } else if(strcmp(word, "start_move") == 0) {
        *out_type = ACTION_TYPE_START_MOVE;
        *dest = NULL;
        return true;

    } else if(strcmp(word, "start_resize") == 0) {
        *out_type = ACTION_TYPE_START_RESIZE;
        *dest = NULL;
        return true;

    } else if(strcmp(word, "repeat_rate") == 0) {
        *out_type = ACTION_TYPE_REPEAT_RATE;
        struct action_repeat_rate *action = ALLOC(struct action_repeat_rate);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->rate)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->delay)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "keymap") == 0) {
        *out_type = ACTION_TYPE_KEYMAP;
        struct action_keymap *action = ALLOC(struct action_keymap);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        action->xkb_layouts = strdup(word);

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        action->xkb_variants = strdup(word);

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        action->xkb_options = strdup(word);
        return true;

    } else if(strcmp(word, "trackpad_disable_while_typing") == 0) {
        *out_type = ACTION_TYPE_TRACKPAD_DISABLE_WHILE_TYPING;
        struct action_trackpad_dwt *action = ALLOC(struct action_trackpad_dwt);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_bool(word, &action->enable)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "trackpad_tap_to_click") == 0) {
        *out_type = ACTION_TYPE_TRACKPAD_TAP_TO_CLICK;
        struct action_trackpad_tap_to_click *action = ALLOC(struct action_trackpad_tap_to_click);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_bool(word, &action->enable)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "trackpad_natural_scroll") == 0) {
        *out_type = ACTION_TYPE_TRACKPAD_NATURAL_SCROLL;
        struct action_trackpad_natural_scroll *action = ALLOC(struct action_trackpad_natural_scroll);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_bool(word, &action->enable)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "trackpad_scroll_method") == 0) {
        *out_type = ACTION_TYPE_TRACKPAD_SCROLL_METHOD;
        struct action_trackpad_scroll_method *action = ALLOC(struct action_trackpad_scroll_method);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_scroll_method(word, &action->value)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "cursor_theme") == 0) {
        *out_type = ACTION_TYPE_CURSOR_THEME;
        struct action_cursor_theme *action = ALLOC(struct action_cursor_theme);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        action->theme = strdup(word);

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->size)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "cursor_warp") == 0) {
        *out_type = ACTION_TYPE_CURSOR_WARP;
        struct action_cursor_warp *action = ALLOC(struct action_cursor_warp);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_cursor_warp(word, &action->value)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "cursor_hide_after_ms") == 0) {
        *out_type = ACTION_TYPE_CURSOR_HIDE_AFTER_MS;
        struct action_cursor_hide_after_ms *action = ALLOC(struct action_cursor_hide_after_ms);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->value)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "gaps") == 0) {
        *out_type = ACTION_TYPE_GAPS;
        struct action_gaps *action = ALLOC(struct action_gaps);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->outer)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }

        if(shell_parser_pop(parser, sizeof(word), word)) {
            if(!parse_int(word, &action->inner)) {
                action_destroy(*out_type, *dest);
                *dest = NULL;
                return false;
            }
        } else {
            // if only a single one is provided we set it for both
            action->inner = action->outer;
        }
        return true;

    } else if(strcmp(word, "smart_gaps") == 0) {
        *out_type = ACTION_TYPE_SMART_GAPS;
        struct action_smart_gaps *action = ALLOC(struct action_smart_gaps);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_bool(word, &action->enable)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "border_width") == 0) {
        *out_type = ACTION_TYPE_BORDER_WIDTH;
        struct action_border_width *action = ALLOC(struct action_border_width);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_int(word, &action->value)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "border_color") == 0) {
        *out_type = ACTION_TYPE_BORDER_COLOR;
        struct action_border_color *action = ALLOC(struct action_border_color);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }

        if(!color_from_hex(word, &action->active)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }

        if(shell_parser_pop(parser, sizeof(word), word)) {
            if(!color_from_hex(word, &action->inactive)) {
                action_destroy(*out_type, *dest);
                *dest = NULL;
                return false;
            }
        } else {
            // both the same
            action->inactive = action->active;
        }
        return true;

    } else if(strcmp(word, "master_ratio") == 0) {
        *out_type = ACTION_TYPE_MASTER_RATIO;
        struct action_master_ratio *action = ALLOC(struct action_master_ratio);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }

        if(strcmp(word, "--adjust") == 0) {
            action->adjust = true;
            if(!shell_parser_pop(parser, sizeof(word), word)) {
                action_destroy(*out_type, *dest);
                *dest = NULL;
                return false;
            }
        }
        if(!parse_float(word, &action->value)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "client_side_decorations") == 0) {
        *out_type = ACTION_TYPE_CLIENT_SIDE_DECORATIONS;
        struct action_csd *action = ALLOC(struct action_csd);
        *dest = action;

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        if(!parse_bool(word, &action->enable)) {
            action_destroy(*out_type, *dest);
            *dest = NULL;
            return false;
        }
        return true;

    } else if(strcmp(word, "pointer_rule") == 0) {
        // TODO: implement pointer_rule
        return false;
    } else if(strcmp(word, "toplevel_rule") == 0) {
        // TODO: implement toplevel_rule
        return false;
    } else if(strcmp(word, "create_keybind") == 0) {
        *out_type = ACTION_TYPE_CREATE_KEYBIND;
        struct keybind *keybind = ALLOC(struct keybind);
        *dest = keybind;

        // NOTE: unlike for other actions, since the `keybind` object is long-lived, `action_destroy()` does not destrot
        // it, so we do that here manually with `FREE()`
        if(!shell_parser_pop(parser, sizeof(word), word)) {
            FREE(keybind);
            *dest = NULL;
            return false;
        }

        if(strcmp(word, "--even_when_locked") == 0) {
            keybind->even_when_locked = true;
            if(!shell_parser_pop(parser, sizeof(word), word)) {
                FREE(keybind);
                *dest = NULL;
                return false;
            }
        }

        if(!parse_modifiers(word, &keybind->modifiers)) {
            FREE(keybind);
            *dest = NULL;
            return false;
        }

        if(!shell_parser_pop(parser, sizeof(word), word)) {
            FREE(keybind);
            *dest = NULL;
            return false;
        }

        if(!parse_keysym(word, &keybind->key)) {
            FREE(keybind);
            *dest = NULL;
            return false;
        }

        // recurse here and create an action from the rest
        if(!action_create(parser, &keybind->type, &keybind->action)) {
            FREE(keybind);
            *dest = NULL;
            return false;
        }

        return true;
    }

    return false;
}
