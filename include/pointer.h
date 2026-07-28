#ifndef POINTER_H
#define POINTER_H

#include <libinput.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_relative_pointer_v1.h>

struct pointer {
    struct wlr_pointer *wlr_pointer;
    struct wl_listener destroy;
    struct wl_list link;
};

struct state;

struct pointer *
pointer_create(struct state *state, struct wlr_pointer *wlr_pointer);

// TODO: figure out
void
pointer_configure(struct pointer *pointer);

#endif
