#ifndef ACTION_TYPES_H
#define ACTION_TYPES_H

// this file, alongside `action_parser.c` is ai generated (and patched if needed) from the specifiaction found in the
// `comphyctl_commands.txt` file. if the file is regenerated this notice should be preserved.

#include <libinput.h>
#include <wlr/types/wlr_output_layout.h>

#include "color.h"

enum action_type {
    ACTION_TYPE_CREATE_WORKSPACE = 1,
    ACTION_TYPE_CHANGE_WORKSPACE,
    ACTION_TYPE_MOVE_TO_WORKSPACE,
    ACTION_TYPE_FOCUS,
    ACTION_TYPE_MOVE,
    ACTION_TYPE_EXEC,
    ACTION_TYPE_ENV,
    ACTION_TYPE_CLOSE,
    ACTION_TYPE_EXIT,
    ACTION_TYPE_TOGGLE_FLOAT,
    ACTION_TYPE_TOGGLE_FULLSCREEN,
    ACTION_TYPE_TOGGLE_FAKE_FULLSCREEN,
    ACTION_TYPE_START_MOVE,
    ACTION_TYPE_START_RESIZE,

    ACTION_TYPE_REPEAT_RATE,
    ACTION_TYPE_KEYMAP,
    ACTION_TYPE_TRACKPAD_DISABLE_WHILE_TYPING,
    ACTION_TYPE_TRACKPAD_TAP_TO_CLICK,
    ACTION_TYPE_TRACKPAD_NATURAL_SCROLL,
    ACTION_TYPE_TRACKPAD_SCROLL_METHOD,
    ACTION_TYPE_CURSOR_THEME,
    ACTION_TYPE_CURSOR_WARP,
    ACTION_TYPE_CURSOR_HIDE_AFTER_MS,
    ACTION_TYPE_GAPS,
    ACTION_TYPE_SMART_GAPS,
    ACTION_TYPE_BORDER_WIDTH,
    ACTION_TYPE_BORDER_COLOR,
    ACTION_TYPE_MASTER_RATIO,

    ACTION_TYPE_TOPLEVEL_RULE,
    ACTION_TYPE_POINTER_RULE,

    ACTION_TYPE_CREATE_KEYBIND,
};

struct action_create_workspace {
    char *output;
    int idx;
};

struct action_change_workspace {
    int idx;
};

struct action_move_to_workspace {
    int idx;
};

struct action_focus {
    enum wlr_direction direction;
};

struct action_move {
    enum wlr_direction direction;
};

struct action_exec {
    char *cmd;
};

struct action_env {
    char *key;
    char *value;
};

struct action_repeat_rate {
    int rate;
    int delay;
};

struct action_keymap {
    char *xkb_layouts;
    char *xkb_variants;
    char *xkb_options;
};

struct action_trackpad_dwt {
    bool enable;
};

struct action_trackpad_tap_to_click {
    bool enable;
};

struct action_trackpad_natural_scroll {
    bool enable;
};

struct action_trackpad_scroll_method {
    enum libinput_config_scroll_method value;
};

struct action_cursor_theme {
    char *theme;
    int size;
};

enum cursor_warp {
    CURSOR_WARP_NONE = 0,
    CURSOR_WARP_ON_FOCUS_CHANGE,
    CURSOR_WARP_ON_OUTPUT_CHANGE,
};

struct action_cursor_warp {
    enum cursor_warp value;
};

struct action_cursor_hide_after_ms {
    int value;
};

struct action_gaps {
    int outer;
    int inner;
};

struct action_smart_gaps {
    bool enable;
};

struct action_border_width {
    int value;
};

struct action_border_color {
    color_t active;
    color_t inactive;
};

struct action_master_ratio {
    bool adjust;
    float value;
};

#endif
