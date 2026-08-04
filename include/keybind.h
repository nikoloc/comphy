#ifndef KEYBIND_H
#define KEYBIND_H

#include <wayland-server-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include "action_types.h"
#include "util/ints.h"

struct keybind {
    u32 modifiers;
    u32 key;
    bool even_when_locked;

    enum action_type type;
    void *action;

    struct wl_list link;
};

struct state;

bool
keybind_check(struct state *state, u32 modifiers, u32 keycode, enum wl_keyboard_key_state key_state);

bool
keybind_check_change_vt(struct state *state, size_t count, const xkb_keysym_t *keysyms);

#endif
