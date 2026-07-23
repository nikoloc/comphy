#include "gamma_control.h"

#include <wlr/types/wlr_output.h>

#include "util/macros.h"

void
handle_set_gamma(struct wl_listener *listener, void *data) {
    struct gamma_control *gamma_control = CONTAINER_OF(listener, struct gamma_control, set_gamma);

    struct wlr_gamma_control_manager_v1_set_gamma_event *event = data;

    struct wlr_output_state state;
    wlr_output_state_init(&state);

    struct wlr_gamma_control_v1 *control =
            wlr_gamma_control_manager_v1_get_control(gamma_control->wlr_gamma_control, event->output);

    if(!wlr_gamma_control_v1_apply(control, &state)) {
        wlr_output_state_finish(&state);
        return;
    }

    if(!wlr_output_commit_state(event->output, &state)) {
        wlr_gamma_control_v1_send_failed_and_destroy(control);
    }

    wlr_output_state_finish(&state);
}

void
gamma_control_init(struct gamma_control *gamma_control, struct wl_display *display) {
    gamma_control->wlr_gamma_control = wlr_gamma_control_manager_v1_create(display);

    gamma_control->set_gamma.notify = handle_set_gamma;
    wl_signal_add(&gamma_control->wlr_gamma_control->events.set_gamma, &gamma_control->set_gamma);
}

void
gamma_control_deinit(struct gamma_control *gamma_control) {
    wl_list_remove(&gamma_control->set_gamma.link);
}
