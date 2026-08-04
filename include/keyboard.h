#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <wlr/types/wlr_keyboard.h>

struct keyboard {
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;

    struct wl_list link;
};

struct state;

struct keyboard *
keyboard_create(struct state *state, struct wlr_keyboard *wlr_keyboard);

void
keyboard_set_repeat_rate(struct keyboard *keyboard, int rate, int delay);

void
keyboard_set_keymap(struct keyboard *keyboard, char *xkb_layouts, char *xkb_variants, char *xkb_options);

#endif
