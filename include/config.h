#ifndef CONFIG_H
#define CONFIG_H

#include <libinput.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include "action.h"
#include "color.h"

struct config {
    struct wl_list keybinds;
    struct wl_list pointer_keybinds;

    struct {
        char *xkb_layouts, *xkb_variants, *xkb_options;
        int rate, delay;
    } keyboard;

    struct wl_list pointer_rules;
    struct wl_list toplevel_rules;

    struct {
        bool disable_while_typing;
        bool natural_scroll;
        bool tap_to_click;
        enum libinput_config_scroll_method scroll_method;
    } trackpad;

    struct {
        char *theme;
        int size;
        int hide_after;
    } cursor;

    struct {
        int width;
        struct {
            color_t active, inactive;
        } color;
    } border;

    struct {
        float active, inactive;
    } opacity;

    struct {
        int outer, inner;
    } gaps;

    float master_ratio;
};

void
config_init(struct config *config);

void
config_deinit(struct config *config);

struct state;

void
config_add_pointer_rule(struct state *state, struct action_pointer *pointer);

void
config_add_toplevel_rule(struct state *state, struct action_toplevel *toplevel);

#endif
