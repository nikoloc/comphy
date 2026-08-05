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

// keybinds are created inside of the `create_keybind` action handler in `action_create()`, but they dont get destroyed
// by the subsequent call to `action_destroy()` since they live in the `state->keybinds` list. thus, we need to destroy
// them explicitly afterwards. NOTE: should only be called on already created keybinds, not if the keybind creation
// fails inside `action_create()`, see also notes there
void
keybind_destroy(struct keybind *keybind);

#endif
