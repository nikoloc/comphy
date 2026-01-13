#include "backend.h"

#include <wlr/render/allocator.h>
#include <wlr/util/log.h>

#include "alloc.h"
#include "macros.h"

static void
handle_new_input(struct wl_listener *listener, void *data) {
    todo();
}

struct backend *
backend_create(struct wl_display *display) {
    struct backend *backend = alloc(sizeof(*backend));
    backend->display = display;

    struct wl_event_loop *loop = wl_display_get_event_loop(display);
    backend->wlr_backend = wlr_backend_autocreate(loop, &backend->wlr_session);
    if(!backend->wlr_backend) {
        goto err;
    }

    backend->wlr_renderer = wlr_renderer_autocreate(backend->wlr_backend);
    if(!backend->wlr_renderer) {
        wlr_log(WLR_ERROR, "failed creating the renderer");
        goto err;
    }

    if(!wlr_renderer_init_wl_display(backend->wlr_renderer, display)) {
        wlr_log(WLR_ERROR, "failed setting up the renderer");
        goto renderer;
    }

    backend->wlr_allocator = wlr_allocator_autocreate(backend->wlr_backend, backend->wlr_renderer);
    if(!backend->wlr_allocator) {
        wlr_log(WLR_ERROR, "failed to create the allocator");
        goto renderer;
    }

    // backend->new_input.notify = handle_new_input;
    // wl_signal_add(&backend->wlr_backend->events.new_input, &backend->new_input);

    return backend;

renderer:
    wlr_renderer_destroy(backend->wlr_renderer);
err:
    free(backend);
    return NULL;
}

void
backend_destroy(struct backend *backend) {
    wlr_backend_destroy(backend->wlr_backend);
    wlr_renderer_destroy(backend->wlr_renderer);
    wlr_allocator_destroy(backend->wlr_allocator);

    free(backend);
}

bool
backend_start(struct backend *backend) {
    return wlr_backend_start(backend->wlr_backend);
}

bool
backend_switch_vt_session(struct backend *backend, int vt) {
    return wlr_session_change_vt(backend->wlr_session, vt);
}
