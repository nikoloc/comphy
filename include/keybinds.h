#ifndef KEYBINDS_H
#define KEYBINDS_H

#include <wayland-server-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include "util/ints.h"

typedef void (*keybind_action_func_t)(void *);

struct keybind {
    bool initialized;
    u32 modifiers;
    u32 key;

    bool active;
    keybind_action_func_t action;
    keybind_action_func_t stop;
    void *args;

    struct wl_list link;
};

bool
keybind_check(u32 modifiers, u32 keycode, enum wl_keyboard_key_state state);

bool
keybind_check_pointer(u32 modifiers, u32 button, enum wl_pointer_button_state state);

bool
keybind_check_change_vt(const xkb_keysym_t *keysyms, size_t count);

void
keybind_stop_server(void *data);

void
keybind_run(void *data);

void
keybind_change_workspace(void *data);

void
keybind_next_workspace(void *data);

void
keybind_prev_workspace(void *data);

void
keybind_move_to_workspace(void *data);

void
keybind_resize(void *data);

void
keybind_stop_resize(void *data);

void
keybind_move(void *data);

void
keybind_stop_move(void *data);

void
keybind_close(void *data);

void
keybind_move_focus(void *data);

void
keybind_swap(void *data);

void
keybind_toggle_floating(void *data);

void
keybind_toggle_fullscreen(void *data);

#endif
