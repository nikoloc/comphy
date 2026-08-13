#include "keyboard.h"

#include <libinput.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "action.h"
#include "keybind.h"
#include "state.h"
#include "util/macros.h"
#include "util/memory.h"

void
keyboard_set_repeat_rate(struct keyboard *keyboard, int rate, int delay) {
    wlr_keyboard_set_repeat_info(keyboard->wlr_keyboard, rate, delay);
}

void
keyboard_set_keymap(struct keyboard *keyboard, char *xkb_layouts, char *xkb_variants, char *xkb_options) {
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if(!context) {
        wlr_log(WLR_ERROR, "could not create xkb context");
        return;
    }

    struct xkb_rule_names rule_names = {
            .layout = xkb_layouts,
            .variant = xkb_variants,
            .options = xkb_options,
    };

    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, &rule_names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if(!keymap) {
        wlr_log(WLR_ERROR, "could not apply the desired configuration to the keyboard");

        keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if(!keymap) {
            wlr_log(WLR_ERROR, "could not apply the default configuration to the keyboard");
            xkb_context_unref(context);
            return;
        }
    }

    wlr_keyboard_set_keymap(keyboard->wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
}

static void
handle_modifiers(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct keyboard *keyboard = CONTAINER_OF(listener, struct keyboard, modifiers);
    struct state *state = state_get();

    wlr_seat_set_keyboard(state->seat.wlr_seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(state->seat.wlr_seat, &keyboard->wlr_keyboard->modifiers);
}

static bool
handle_change_vt(struct state *state, int count, const xkb_keysym_t *keysyms) {
    // from `labwc`, thanks!
    for(int i = 0; i < count; i++) {
        int vt = keysyms[i] - XKB_KEY_XF86Switch_VT_1 + 1;
        if(vt >= 1 && vt <= 12) {
            backend_change_vt(&state->backend, vt);
            return true;
        }
    }

    return false;
}

static bool
handle_keybinds(struct state *state, struct keyboard *keyboard, u32 keycode, int count, const u32 *syms,
        enum wl_keyboard_key_state key_state) {
    // for reference see notes on key consumption in the `xkbcommon.h` header
    u32 mods = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
    u32 consumed_mods = xkb_state_key_get_consumed_mods(keyboard->wlr_keyboard->xkb_state, keycode);
    static const u32 significant_mods = WLR_MODIFIER_SHIFT | WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT | WLR_MODIFIER_MOD2 |
                                        WLR_MODIFIER_MOD3 | WLR_MODIFIER_LOGO | WLR_MODIFIER_MOD5;

    for(int i = 0; i < count; i++) {
        struct keybind *iter;
        wl_list_for_each(iter, &state->keybinds, link) {
            if(syms[i] == iter->key && ((mods & ~consumed_mods & significant_mods) == iter->modifiers) &&
                    key_state == WL_KEYBOARD_KEY_STATE_PRESSED) {
                wlr_log(WLR_ERROR, "action: %d", iter->type);
                action_perform(state, iter->type, iter->action);
                return true;
            }
        }
    }

    return false;
}

static void
handle_key(struct wl_listener *listener, void *data) {
    struct keyboard *keyboard = CONTAINER_OF(listener, struct keyboard, key);
    struct wlr_keyboard_key_event *event = data;
    struct state *state = state_get();

    if(state->operation && state->operation_server_inited) {
        operation_stop_whatever(state);
    }

    // translate libinput keycode -> xkbcommon
    u32 keycode = event->keycode + 8;

    const xkb_keysym_t *syms;
    int count = xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = handle_change_vt(state, count, syms);
    if(!handled) {
        handled = handle_keybinds(state, keyboard, keycode, count, syms, event->state);
    }
    if(!handled) {
        // pass to client
        wlr_seat_set_keyboard(state->seat.wlr_seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(state->seat.wlr_seat, event->time_msec, event->keycode, event->state);
    }
}

static void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct keyboard *keyboard = CONTAINER_OF(listener, struct keyboard, destroy);

    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->destroy.link);
    wl_list_remove(&keyboard->link);

    free(keyboard);
}

struct keyboard *
keyboard_create(struct state *state, struct wlr_keyboard *wlr_keyboard) {
    struct keyboard *keyboard = ALLOC(struct keyboard);
    keyboard->wlr_keyboard = wlr_keyboard;
    wlr_keyboard->data = keyboard;

    // initial configuration
    keyboard_set_repeat_rate(keyboard, state->config.keyboard.rate, state->config.keyboard.delay);
    keyboard_set_keymap(keyboard, state->config.keyboard.xkb_layouts, state->config.keyboard.xkb_variants,
            state->config.keyboard.xkb_options);

    wlr_seat_set_keyboard(state->seat.wlr_seat, keyboard->wlr_keyboard);

    wl_list_insert(&state->keyboards, &keyboard->link);

    keyboard->modifiers.notify = handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);

    keyboard->key.notify = handle_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);

    keyboard->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_keyboard->base.events.destroy, &keyboard->destroy);

    return keyboard;
}
