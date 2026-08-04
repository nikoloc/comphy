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
        bool dwt;
        bool natural_scroll;
        bool tap_to_click;
        enum libinput_config_scroll_method scroll_method;
    } trackpad;

    struct {
        int hide_after_ms;
        enum cursor_warp warp;
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
        bool smart;
    } gaps;

    float master_ratio;
};

void
config_init(struct config *config);

void
config_deinit(struct config *config);

struct state;

#endif
