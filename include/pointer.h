#ifndef POINTER_H
#define POINTER_H

#include <libinput.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_relative_pointer_v1.h>

#include "action.h"

struct pointer {
    struct wlr_pointer *wlr_pointer;
    struct wl_listener destroy;
    struct wl_list link;
};

struct state;

struct pointer *
pointer_create(struct state *state, struct wlr_pointer *wlr_pointer);

const char *
pointer_get_name(struct pointer *pointer);

void
pointer_configure(struct pointer *pointer, float sensitivity, enum tri_state acceleration, enum tri_state left_handed);

void
pointer_configure_if_trackpad(struct pointer *pointer, enum tri_state tap_to_click, enum tri_state disable_while_typing,
        enum tri_state natural_scroll, enum libinput_config_scroll_method scroll_method);

#endif
