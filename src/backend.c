#include "backend.h"

#include <signal.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/util/log.h>

#include "keyboard.h"
#include "output.h"
#include "pointer.h"
#include "seat.h"
#include "state.h"
#include "util/ints.h"
#include "util/macros.h"

static int
handle_sigint(int signo, void *data) {
    UNUSED(signo), UNUSED(data);

    // TODO: check if setting `state->is_exiting` is needed
    struct state *state = state_get();
    wl_display_terminate(state->display);

    return 0;
}

bool
backend_init(struct backend *backend, struct wl_display *display) {
    struct wl_event_loop *event_loop = wl_display_get_event_loop(display);

    backend->wlr_backend = wlr_backend_autocreate(event_loop, &backend->session);
    if(!backend->wlr_backend) {
        wlr_log(WLR_ERROR, "could not create the backend");
        goto err;
    }

    if(!backend->session) {
        wlr_log(WLR_INFO, "could not setup the session, non critical");
    }

    backend->renderer = wlr_renderer_autocreate(backend->wlr_backend);
    if(!backend->renderer) {
        wlr_log(WLR_ERROR, "could not create the renderer");
        goto backend;
    }

    bool success = wlr_renderer_init_wl_display(backend->renderer, display);
    if(!success) {
        wlr_log(WLR_ERROR, "could not setup the renderer");
        goto renderer;
    }

    backend->allocator = wlr_allocator_autocreate(backend->wlr_backend, backend->renderer);
    if(!backend->allocator) {
        wlr_log(WLR_ERROR, "could not create the allocator");
        goto renderer;
    }

    // add signal handlers
    backend->sigint_source = wl_event_loop_add_signal(event_loop, SIGINT, handle_sigint, NULL);

    return true;

renderer:
    wlr_renderer_destroy(backend->renderer);
backend:
    wlr_backend_destroy(backend->wlr_backend);
err:
    return false;
}

static void
handle_new_output(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_output *wlr_output = data;
    struct state *state = state_get();

    output_create(state, wlr_output);
}

static void
handle_new_input(struct wl_listener *listener, void *data) {
    UNUSED(listener);

    struct wlr_input_device *device = data;
    struct state *state = state_get();

    switch(device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD: {
            struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);
            keyboard_create(state, wlr_keyboard);
            break;
        }
        case WLR_INPUT_DEVICE_POINTER: {
            struct wlr_pointer *wlr_pointer = wlr_pointer_from_input_device(device);
            pointer_create(state, wlr_pointer);
            break;
        }
        default: {
            // not supported
            break;
        }
    }

    seat_inform_capabilities(state);
}

bool
backend_start(struct backend *backend) {
    // here we wire up the listeners and start the underlying wlr_backend. note: we need to wire up listeners here,
    // since they might fire before we actually start the event loop
    backend->new_input.notify = handle_new_input;
    wl_signal_add(&backend->wlr_backend->events.new_input, &backend->new_input);

    backend->new_output.notify = handle_new_output;
    wl_signal_add(&backend->wlr_backend->events.new_output, &backend->new_output);

    return wlr_backend_start(backend->wlr_backend);
}

bool
backend_change_vt(struct backend *backend, u32 vt) {
    // there might not be session inited if the backend is not drm/kms one (e.g. wayland backend)
    if(!backend->session) {
        return false;
    }

    return wlr_session_change_vt(backend->session, vt);
}

void
backend_deinit(struct backend *backend) {
    // remove listeners
    wl_list_remove(&backend->new_output.link);
    wl_list_remove(&backend->new_input.link);

    wl_event_source_remove(backend->sigint_source);

    wlr_allocator_destroy(backend->allocator);
    wlr_renderer_destroy(backend->renderer);
    wlr_backend_destroy(backend->wlr_backend);
}
