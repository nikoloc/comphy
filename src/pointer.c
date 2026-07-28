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
#include "dnd.h"
#include "keybinds.h"
#include "layer.h"
#include "layout.h"
#include "output.h"
#include "toplevel.h"
#include "util/macros.h"
#include "util/memory.h"
#include "workspace.h"

void
handle_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);

    struct pointer *pointer = CONTAINER_OF(listener, struct pointer, destroy);
    wl_list_remove(&pointer->link);
    wl_list_remove(&pointer->destroy.link);
    free(pointer);
}

struct pointer *
pointer_create(struct state *state, struct wlr_pointer *wlr_pointer) {
    struct pointer *pointer = ALLOCATE(struct pointer);

    pointer->wlr_pointer = wlr_pointer;
    wlr_pointer->data = pointer;

    pointer_configure(pointer);

    wlr_cursor_attach_input_device(state->cursor.wlr_cursor, &wlr_pointer->base);

    wl_list_insert(&state->pointers, &pointer->link);

    pointer->destroy.notify = handle_destroy;
    wl_signal_add(&wlr_pointer->base.events.destroy, &pointer->destroy);

    return pointer;
}

// bool
// pointer_configure(struct mwc_pointer *pointer) {
//   if(!wlr_input_device_is_libinput(&pointer->wlr_pointer->base)) return false;
// wlr_log(WLR_ERROR, "could not configure pointer device");
//
//   struct libinput_device *device = wlr_libinput_get_device_handle(&pointer->wlr_pointer->base);
//   libinput_device_ref(device);
//   pointer->name = libinput_device_get_name(device);
//
//   enum libinput_config_accel_profile accel;
//   double sensitivity;
//   /* we configure accelation and sensitivity of the pointer by
//    * first looking at specific pointer configurations */
//   bool found = false;
//   struct pointer_config *p;
//   wl_list_for_each(p, &server.config->pointers, link) {
//     if(strcmp(p->name, pointer->name) == 0) {
//       accel = p->acceleration;
//       sensitivity = p->sensitivity;
//       found = true;
//       break;
//     }
//   }
//
//   if(!found) {
//     accel = server.config->pointer_acceleration;
//     sensitivity = server.config->pointer_sensitivity;
//   }
//
//   if(libinput_device_config_accel_is_available(device)) {
//     if(libinput_device_config_accel_set_speed(device, sensitivity)
//        != LIBINPUT_CONFIG_STATUS_SUCCESS) {
//       wlr_log(WLR_ERROR, "applying sensitivity to device '%s' failed", pointer->name);
//     }
//
//     if(accel) {
//       struct libinput_config_accel *accel_config = libinput_config_accel_create(accel);
//       if(libinput_device_config_accel_apply(device, accel_config)
//          != LIBINPUT_CONFIG_STATUS_SUCCESS) {
//         wlr_log(WLR_ERROR, "applying acceleration profile to device '%s' failed", pointer->name);
//       }
//       libinput_config_accel_destroy(accel_config);
//     }
//   }
//
//   /* check if trackpad */
//   if(libinput_device_config_tap_get_finger_count(device) > 0) {
//     /* then apply trackpad specific settings */
//     if(libinput_device_config_tap_set_enabled(device, server.config->trackpad_tap_to_click)
//        != LIBINPUT_CONFIG_STATUS_SUCCESS) {
//       wlr_log(WLR_ERROR, "applying tap to click to device '%s' failed", pointer->name);
//     }
//
//     if(libinput_device_config_scroll_set_natural_scroll_enabled(device,
//        server.config->trackpad_natural_scroll) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
//       wlr_log(WLR_ERROR, "applying natural scroll to device '%s' failed", pointer->name);
//     }
//
//     if(libinput_device_config_scroll_set_method(device, server.config->trackpad_scroll_method)
//        != LIBINPUT_CONFIG_STATUS_SUCCESS) {
//       wlr_log(WLR_ERROR, "applying scroll method to device '%s' failed", pointer->name);
//     }
//
//     if(libinput_device_config_dwt_set_enabled(device,
//        server.config->trackpad_disable_while_typing) != LIBINPUT_CONFIG_STATUS_SUCCESS) {
//       wlr_log(WLR_ERROR, "applying disable while typing to device '%s' failed", pointer->name);
//     }
//   }
//
//   libinput_device_unref(device);
//
//   return true;
// }
//
