#include "pointer.h"

#include <libinput.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/util/log.h>
#include <wlr/util/region.h>

#include "config.h"
#include "rules.h"
#include "state.h"
#include "toplevel.h"
#include "util/macros.h"
#include "util/memory.h"

void
pointer_if_trackpad_set_tap_to_click(struct pointer *pointer, bool tap_to_click) {
    if(!wlr_input_device_is_libinput(&pointer->wlr_pointer->base)) {
        wlr_log(WLR_ERROR, "pointer device is not libinput");
        return;
    }

    struct libinput_device *dev = wlr_libinput_get_device_handle(&pointer->wlr_pointer->base);
    libinput_device_ref(dev);

    if(libinput_device_config_tap_get_finger_count(dev) <= 0) {
        // not a trackpad
        libinput_device_unref(dev);
        return;
    }

    const char *name = libinput_device_get_name(dev);

    if(libinput_device_config_tap_set_enabled(dev, tap_to_click) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
        wlr_log(WLR_ERROR, "applying tap to click to device '%s' failed", name);
    }

    libinput_device_unref(dev);
}

void
pointer_if_trackpad_set_dwt(struct pointer *pointer, bool dwt) {
    if(!wlr_input_device_is_libinput(&pointer->wlr_pointer->base)) {
        wlr_log(WLR_ERROR, "pointer device is not libinput");
        return;
    }

    struct libinput_device *dev = wlr_libinput_get_device_handle(&pointer->wlr_pointer->base);
    libinput_device_ref(dev);

    if(libinput_device_config_tap_get_finger_count(dev) <= 0) {
        // not a trackpad
        libinput_device_unref(dev);
        return;
    }

    const char *name = libinput_device_get_name(dev);

    if(libinput_device_config_dwt_set_enabled(dev, dwt) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
        wlr_log(WLR_ERROR, "applying disable while typing to device '%s' failed", name);
    }

    libinput_device_unref(dev);
}

void
pointer_if_trackpad_set_natural_scroll(struct pointer *pointer, bool natural_scroll) {
    if(!wlr_input_device_is_libinput(&pointer->wlr_pointer->base)) {
        wlr_log(WLR_ERROR, "pointer device is not libinput");
        return;
    }

    struct libinput_device *dev = wlr_libinput_get_device_handle(&pointer->wlr_pointer->base);
    libinput_device_ref(dev);

    if(libinput_device_config_tap_get_finger_count(dev) <= 0) {
        // not a trackpad
        libinput_device_unref(dev);
        return;
    }

    const char *name = libinput_device_get_name(dev);

    if(libinput_device_config_scroll_set_natural_scroll_enabled(dev, natural_scroll) !=
            LIBINPUT_CONFIG_STATUS_SUCCESS) {
        wlr_log(WLR_ERROR, "applying natural scroll to device '%s' failed", name);
    }

    libinput_device_unref(dev);
}

void
pointer_if_trackpad_set_scroll_method(struct pointer *pointer, enum libinput_config_scroll_method scroll_method) {
    if(!wlr_input_device_is_libinput(&pointer->wlr_pointer->base)) {
        wlr_log(WLR_ERROR, "pointer device is not libinput");
        return;
    }

    struct libinput_device *dev = wlr_libinput_get_device_handle(&pointer->wlr_pointer->base);
    libinput_device_ref(dev);

    if(libinput_device_config_tap_get_finger_count(dev) <= 0) {
        // not a trackpad
        libinput_device_unref(dev);
        return;
    }

    const char *name = libinput_device_get_name(dev);

    if(libinput_device_config_scroll_set_method(dev, scroll_method) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
        wlr_log(WLR_ERROR, "applying scroll method to device '%s' failed", name);
    }

    libinput_device_unref(dev);
}

static void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct pointer *pointer = CONTAINER_OF(listener, struct pointer, destroy);
    wl_list_remove(&pointer->link);
    wl_list_remove(&pointer->destroy.link);
    free(pointer);
}

struct pointer *
pointer_create(struct state *state, struct wlr_pointer *wlr_pointer) {
    struct pointer *pointer = ALLOC(struct pointer);

    pointer->wlr_pointer = wlr_pointer;
    wlr_pointer->data = pointer;

    wlr_cursor_attach_input_device(state->cursor.wlr_cursor, &wlr_pointer->base);
    pointer_configure_from_rules(state, pointer);

    wl_list_insert(&state->pointers, &pointer->link);

    pointer->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_pointer->base.events.destroy, &pointer->destroy);

    return pointer;
}

static bool
matches_rule(const char *name, struct pointer_rule *rule) {
    if((rule->fields & POINTER_RULE_FIELD_MATCH_NAME) && rule->match.name && !strstr(name, rule->match.name)) {
        return false;
    }

    return true;
}

static void
create_config(struct state *state, const char *name, struct pointer_rule *config) {
    struct pointer_rule *iter;
    wl_list_for_each(iter, &state->config.pointer_rules, link) {
        if(!matches_rule(name, iter)) {
            continue;
        }

        if(iter->fields & POINTER_RULE_FIELD_SENSITIVITY) {
            config->sensitivity = iter->sensitivity;
        }

        if(iter->fields & POINTER_RULE_FIELD_ACCELERATION) {
            config->acceleration = iter->acceleration;
        }

        if(iter->fields & POINTER_RULE_FIELD_LEFT_HANDED) {
            config->left_handed = iter->left_handed;
        }
    }
}

void
pointer_configure_from_rules(struct state *state, struct pointer *pointer) {
    if(!wlr_input_device_is_libinput(&pointer->wlr_pointer->base)) {
        wlr_log(WLR_ERROR, "could not configure pointer device");
        return;
    }

    struct libinput_device *dev = wlr_libinput_get_device_handle(&pointer->wlr_pointer->base);
    libinput_device_ref(dev);

    const char *name = libinput_device_get_name(dev);

    struct pointer_rule config = {0};
    create_config(state, name, &config);

    if(libinput_device_config_accel_is_available(dev)) {
        if(libinput_device_config_accel_set_speed(dev, config.sensitivity) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
            wlr_log(WLR_ERROR, "applying sensitivity to device '%s' failed", name);
        }

        enum libinput_config_accel_profile accel_profile =
                config.acceleration ? LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE : LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT;
        struct libinput_config_accel *accel_config = libinput_config_accel_create(accel_profile);

        if(libinput_device_config_accel_apply(dev, accel_config) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
            wlr_log(WLR_ERROR, "applying acceleration profile to device '%s' failed", name);
        }
        libinput_config_accel_destroy(accel_config);
    } else {
        wlr_log(WLR_ERROR, "acceleration and sensitivity options not available to device '%s'", name);
    }

    if(libinput_device_config_left_handed_is_available(dev)) {
        if(libinput_device_config_left_handed_set(dev, config.left_handed) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
            wlr_log(WLR_ERROR, "applying left handed to device '%s' failed", name);
        }
    } else {
        wlr_log(WLR_ERROR, "left handed option not availabe to device '%s'", name);
    }

    libinput_device_unref(dev);
}
