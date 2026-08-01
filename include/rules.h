#ifndef RULES_H
#define RULES_H

#include <stdbool.h>

#include "toplevel.h"
#include "util/ints.h"

enum pointer_rule_field {
    POINTER_RULE_FIELD_MATCH_NAME = (1 << 0),

    POINTER_RULE_FIELD_SENSITIVITY = (1 << 1),
    POINTER_RULE_FIELD_ACCELERATION = (1 << 2),
    POINTER_RULE_FIELD_LEFT_HANDED = (1 << 3),
};

struct pointer_rule {
    u32 fields;

    struct {
        char *name;
    } match;

    float sensitivity;
    bool acceleration;
    bool left_handed;
};

enum toplevel_rule_field {
    TOPLEVEL_RULE_FIELD_MATCH_APP_ID = (1 << 0),
    TOPLEVEL_RULE_FIELD_MATCH_TITLE = (1 << 1),
    TOPLEVEL_RULE_FIELD_MATCH_STATE = (1 << 2),

    TOPLEVEL_RULE_FIELD_CLIENT_SIDE_DECORATIONS = (1 << 3),
    TOPLEVEL_RULE_FIELD_OPACITY_ACTIVE = (1 << 4),
    TOPLEVEL_RULE_FIELD_OPACITY_INACTIVE = (1 << 5),
    TOPLEVEL_RULE_FIELD_DEFAULT_STATE = (1 << 6),
    TOPLEVEL_RULE_FIELD_DEFAULT_WIDTH = (1 << 7),
    TOPLEVEL_RULE_FIELD_DEFAULT_HEIGHT = (1 << 8),
};

struct toplevel_rule {
    u32 fields;

    struct {
        char *app_id;
        char *title;
        enum toplevel_state state;
    } match;

    bool client_side_decorations;
    float opacity_active;
    float opacity_inactive;
    enum toplevel_state default_state;
    i32 default_width;
    i32 default_height;
};

#endif
