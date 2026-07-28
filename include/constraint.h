#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>

struct constraint {
    struct wlr_pointer_constraint_v1 *wlr_constraint;

    struct wl_listener destroy;
};

struct state;

void
constraint_apply_to_move(struct constraint *constraint, double *dx, double *dy);

void
constraint_remove_current(void);

void
constraint_set_as_current(struct state *state, struct constraint *constraint);

void
constraint_move_to_hint(struct state *state, struct constraint *constraint);

#endif
