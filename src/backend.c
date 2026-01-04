#include "backend.h"

#include "alloc.h"
#include "macros.h"

static void
handle_new_output(struct wl_listener *listener, void *data) {
    todo();
}

static void
handle_new_input(struct wl_listener *listener, void *data) {
    todo();
}

struct backend *
backend_create(struct wl_display *display) {
    struct backend *backend = alloc(sizeof(*backend));

    struct wl_event_loop *loop = wl_display_get_event_loop(display);
    backend->wlr_backend = wlr_backend_autocreate(loop, &backend->wlr_session);
    if(!backend->wlr_backend) {
        free(backend);
        return NULL;
    }

    wl_list_init(&backend->outputs);

    backend->new_output.notify = handle_new_output;
    wl_signal_add(&backend->wlr_backend->events.new_output, &backend->new_output);

    backend->new_input.notify = handle_new_input;
    wl_signal_add(&backend->wlr_backend->events.new_input, &backend->new_input);

    return backend;
}

void
backend_destroy(struct backend *backend) {
    wlr_backend_destroy(backend->wlr_backend);

    free(backend);
}

bool
backend_switch_vt_session(struct backend *backend, int vt) {
    return wlr_session_change_vt(backend->wlr_session, vt);
}
