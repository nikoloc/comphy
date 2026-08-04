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
pointer_configure_from_rules(struct state *state, struct pointer *pointer);

void
pointer_if_trackpad_set_tap_to_click(struct pointer *pointer, bool tap_to_click);

void
pointer_if_trackpad_set_dwt(struct pointer *pointer, bool dwt);

void
pointer_if_trackpad_set_natural_scroll(struct pointer *pointer, bool natural_scroll);

void
pointer_if_trackpad_set_scroll_method(struct pointer *pointer, enum libinput_config_scroll_method scroll_method);

#endif
