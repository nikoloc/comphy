#ifndef GAMMA_CONTROL_H
#define GAMMA_CONTROL_H

#include <wlr/types/wlr_gamma_control_v1.h>

struct gamma_control {
    struct wlr_gamma_control_manager_v1 *wlr_gamma_control;

    struct wl_listener set_gamma;
};

void
gamma_control_init(struct gamma_control *gamma_control, struct wl_display *display);

void
gamma_control_deinit(struct gamma_control *gamma_control);

#endif
