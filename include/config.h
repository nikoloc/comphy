#ifndef CONFIG_H
#define CONFIG_H

#include <libinput.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include "color.h"
#include "util/dstring.h"

struct window_rule_regex {
    bool has_app_id_regex;
    regex_t app_id_regex;
    bool has_title_regex;
    regex_t title_regex;
};

struct window_rule_float {
    struct window_rule_regex condition;

    struct wl_list link;
};

struct window_rule_size {
    struct window_rule_regex condition;

    bool is_relative_width, is_relative_height;
    int width, height;

    struct wl_list link;
};

struct window_rule_opacity {
    struct window_rule_regex condition;

    float inactive_value, active_value;

    struct wl_list link;
};

struct workspace_config {
    int index;
    char *output;

    struct wl_list link;
};

struct pointer_config {
    char *name;
    double sensitivity;
    enum libinput_config_accel_profile acceleration;

    double pointer_sensitivity;
    bool pointer_acceleration;
    bool pointer_left_handed;

    struct wl_list link;
};

struct config {
    struct wl_list keybinds;
    struct wl_list pointer_keybinds;

    struct wl_list workspaces;

    struct {
        struct wl_list floating;
        struct wl_list size;
        struct wl_list opacity;
    } window_rules;

    string_t keymap_layouts;
    string_t keymap_variants;
    string_t keymap_options;
    int keyboard_rate;
    int keyboard_delay;

    struct wl_list pointers;

    bool trackpad_disable_while_typing;
    bool trackpad_natural_scroll;
    bool trackpad_tap_to_click;
    enum libinput_config_scroll_method trackpad_scroll_method;

    string_t cursor_theme;
    int cursor_size;

    color_t active_border_color, inactive_border_color;
    double active_opacity, inactive_opacity;

    int border_width;
    int outer_gaps, inner_gaps;

    double master_ratio;
    bool client_side_decorations;
};

void
config_init(struct config *config);

// TODO: make these take already parsed params, or something
bool
config_add_window_rule(struct config *conf, char *app_id_regex, char *title_regex, char *predicate, char **args,
        size_t arg_count);

bool
config_add_keybind(struct config *c, char *modifiers, char *key, char *action, char **args, size_t arg_count);

void
config_deinit(struct config *config);

#endif
