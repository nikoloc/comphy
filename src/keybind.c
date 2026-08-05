#include "keybind.h"

#include <stddef.h>
#include <stdint.h>
#include <wayland-util.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/xcursor.h>

#include "action.h"
#include "util/memory.h"

void
keybind_destroy(struct keybind *keybind) {
    action_destroy(keybind->type, keybind->action);
    wl_list_remove(&keybind->link);

    FREE(keybind);
}
