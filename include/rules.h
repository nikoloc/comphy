#ifndef RULES_H
#define RULES_H

#include <stdbool.h>
#include <wayland-util.h>

#include "toplevel.h"
#include "util/ints.h"

// all the rules in `comphy` are static, meaning they are not going to be reevaluated on any of the matched state
// changing, but instead only when needed.

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

    struct wl_list link;
};

enum toplevel_rule_field {
    TOPLEVEL_RULE_FIELD_MATCH_APP_ID = (1 << 0),
    TOPLEVEL_RULE_FIELD_MATCH_TITLE = (1 << 1),

    TOPLEVEL_RULE_FIELD_STATE = (1 << 2),
    TOPLEVEL_RULE_FIELD_WIDTH = (1 << 3),
    TOPLEVEL_RULE_FIELD_HEIGHT = (1 << 4),
};

struct toplevel_rule {
    u32 fields;

    struct {
        char *app_id, *title;
        enum toplevel_state state;
    } match;

    enum toplevel_state state;
    int width, height;

    struct wl_list link;
};

enum output_rule_field {
    OUTPUT_RULE_FIELD_MATCH_NAME = (1 << 0),

    OUTPUT_RULE_FIELD_X = (1 << 1),
    OUTPUT_RULE_FIELD_Y = (1 << 2),
    OUTPUT_RULE_FIELD_WIDTH = (1 << 3),
    OUTPUT_RULE_FIELD_HEIGHT = (1 << 4),
    OUTPUT_RULE_FIELD_REFRESH_RATE = (1 << 5),
    OUTPUT_RULE_FIELD_SCALE = (1 << 6),
};

struct output_rule {
    u32 fields;

    struct {
        char *name;
    } match;

    int x, y, width, height;
    int refresh_rate;
    float scale;

    struct wl_list link;
};

// similar note as for the `keybind_destroy()`
void
toplevel_rule_destroy(struct toplevel_rule *rule);

void
pointer_rule_destroy(struct pointer_rule *rule);

void
output_rule_destroy(struct output_rule *rule);

#endif
