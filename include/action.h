#ifndef ACTION_H
#define ACTION_H

#include <libinput.h>
#include <wlr/types/wlr_output_layout.h>

#include "color.h"
#include "toplevel.h"
#include "util/ints.h"

enum action_type {
    ACTION_TYPE_CREATE_WORKSPACE = 1,
    ACTION_TYPE_CHANGE_WORKSPACE,
    ACTION_TYPE_MOVE_TO_WORKSPACE,
    ACTION_TYPE_FOCUS,
    ACTION_TYPE_MOVE,
    ACTION_TYPE_EXEC,
    ACTION_TYPE_ENV,
    ACTION_TYPE_KEYBOARD,
    ACTION_TYPE_POINTER,
    ACTION_TYPE_TRACKPAD,
    ACTION_TYPE_CURSOR,
    ACTION_TYPE_GAPS,
    ACTION_TYPE_BORDER,
    ACTION_TYPE_TOPLEVEL,
    ACTION_TYPE_ADJUST_MASTER_RATIO,
    ACTION_TYPE_SET_MASTER_RATIO,
    ACTION_TYPE_CREATE_KEYBINDS,
};

struct action_create_workspace {
    char *output;

    i32 idx;
};

struct action_change_workspace {
    i32 idx;
};

struct action_move_to_workspace {
    i32 idx;
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

struct action_keyboard {
    i32 rate;
    i32 delay;

    char *xkb_layouts;
    char *xkb_variants;
    char *xkb_options;
};

struct action_pointer {
    char *name;
    float sensitivity;
    bool acceleration;
    bool left_handed;
};

struct action_trackpad {
    bool disable_while_typing;
    bool natural_scroll;
    enum libinput_config_scroll_method scroll_method;
};

struct action_cursor {
    char *theme;
    i32 size;
    bool warp_on_output_change;
    i32 hide_after;
};

struct action_gaps {
    bool smart;
    i32 outer;
    i32 inner;
};

struct action_border {
    i32 width;
    color_t color_active;
    color_t color_inactive;
};

struct action_toplevel {
    char *app_id;
    char *title;
    bool client_side_decoration;
    float opacity_active;
    float opacity_inactive;
    enum toplevel_state default_state;
    i32 default_width;
    i32 default_height;
};

struct action_adjust_master_ratio {
    float value;
};

struct action_set_master_ratio {
    float value;
};

struct action_create_keybinds {
    bool even_when_locked;
    u32 modifiers;
    u32 key;

    enum action_type type;
    void *action;
};

void
action_perform(struct state *state, enum action_type type, void *action);

#endif
