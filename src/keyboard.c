#include "keyboard.h"

#include <libinput.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "state.h"
#include "util/macros.h"
#include "util/memory.h"

void
keyboard_configure(struct keyboard *keyboard, int rate, int delay, char *xkb_layouts, char *xkb_variants,
        char *xkb_options) {
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
        if(keymap) {
            wlr_log(WLR_ERROR, "could not apply the default configuration to the keyboard");
            xkb_context_unref(context);
            return;
        }
    }

    wlr_keyboard_set_keymap(keyboard->wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(keyboard->wlr_keyboard, rate, delay);
}

static void
handle_modifiers(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct keyboard *keyboard = CONTAINER_OF(listener, struct keyboard, modifiers);
    struct state *state = state_get();

    // TODO: i dont think we need this here since we also get the key event for it
    state->active_keyboard = keyboard;

    wlr_seat_set_keyboard(state->seat.wlr_seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(state->seat.wlr_seat, &keyboard->wlr_keyboard->modifiers);
}

void
handle_key(struct wl_listener *listener, void *data) {
    struct keyboard *keyboard = CONTAINER_OF(listener, struct keyboard, modifiers);
    struct wlr_keyboard_key_event *event = data;
    struct state *state = state_get();

    state->active_keyboard = keyboard;

    // translate libinput keycode -> xkbcommon
    u32 keycode = event->keycode + 8;

    const xkb_keysym_t *syms;
    int count = xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    // TODO: when keybinds
    // bool handled = handle_change_vt_key(syms, count);
    // if(!handled) {
    //     handled = server_handle_keybinds(keyboard, keycode, event->state);
    // }

    if(!handled) {
        // pass to client
        wlr_seat_set_keyboard(state->seat.wlr_seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(state->seat.wlr_seat, event->time_msec, event->keycode, event->state);
    }
}

void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct keyboard *keyboard = CONTAINER_OF(listener, struct keyboard, modifiers);
    struct state *state = state_get();

    if(state->active_keyboard == keyboard) {
        state->active_keyboard = NULL;
    }

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

    keyboard_configure(keyboard, state->config.keyboard.rate, state->config.keyboard.delay,
            state->config.keyboard.xkb_layouts, state->config.keyboard.xkb_variants,
            state->config.keyboard.xkb_options);

    wlr_seat_set_keyboard(state->seat.wlr_seat, keyboard->wlr_keyboard);
    if(!state->active_keyboard) {
        state->active_keyboard = keyboard;
    }

    wl_list_insert(&state->keyboards, &keyboard->link);

    keyboard->modifiers.notify = handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);

    keyboard->key.notify = handle_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);

    keyboard->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_keyboard->base.events.destroy, &keyboard->destroy);

    return keyboard;
}
