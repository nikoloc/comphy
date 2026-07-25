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

struct pointer_constraint {
    struct wlr_pointer_constraint_v1 *wlr_pointer_constraint;

    struct wl_listener destroy;
};

struct state;

struct pointer *
pointer_create(struct state *state, struct wlr_pointer *wlr_pointer);

bool
pointer_configure(struct pointer *pointer);

void
pointer_handle_focus(uint32_t time, bool handle_keyboard_focus);

// move to other file
void
pointer_constraint_apply_to_move(double *dx, double *dy);

void
pointer_constraint_remove_current(void);

void
pointer_constraint_set_as_current(struct pointer_constraint *constraint);

void
pointer_constraint_move_to_hint(struct pointer_constraint *constraint);

#endif
