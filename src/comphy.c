#include "comphy.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_fractional_scale_v1.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/types/wlr_virtual_pointer_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/util/log.h>

#include "backend.h"
#include "config.h"
#include "cursor.h"
#include "decoration.h"
#include "gamma_control.h"
#include "scene.h"
#include "seat.h"
#include "state.h"
#include "util/macros.h"

void
create_temp_dir(void) {
    mkdir("/tmp/mwc", 0777);
}

void
init_logs(bool debug) {
    if(debug) {
        // make it so all the logs do to the log file
        FILE *file = fopen("/tmp/mwc/logs", "w");
        if(file) {
            int fd = fileno(file);
            close(1);
            close(2);

            dup2(fd, 1);
            dup2(fd, 2);

            fclose(file);
        }

        wlr_log_init(WLR_DEBUG, NULL);
    } else {
        wlr_log_init(WLR_INFO, NULL);
    }
}

int
main(int argc, char *argv[]) {
    UNUSED(argc), UNUSED(argv);

    create_temp_dir();
    init_logs(true);

    struct state *state = state_get();
    state->display = wl_display_create();
    if(!state->display) {
        wlr_log(WLR_ERROR, "could not create the display");
        goto err;
    }

    config_init(&state->config);
    if(!backend_init(&state->backend, state->display)) {
        wlr_log(WLR_ERROR, "could not create the backend");
        goto config;
    }

    state->output_layout = wlr_output_layout_create(state->display);

    scene_init(&state->scene, state->output_layout);
    seat_init(&state->seat, state->display);
    xdg_shell_init(&state->xdg_shell, state->display);
    layer_shell_init(&state->layer_shell, state->display);
    cursor_init(&state->cursor, state->output_layout);
    decoration_init(&state->decoration, state->display);
    gamma_control_init(&state->gamma_control, state->display);

    state->foreign_toplevel_manager = wlr_foreign_toplevel_manager_v1_create(state->display);

    wl_list_init(&state->pointers);
    wl_list_init(&state->keyboards);

    // essensial interfaces
    wlr_compositor_create(state->display, 6, state->backend.renderer);
    wlr_subcompositor_create(state->display);
    // other hands off interfaces provided by wlroots
    wlr_data_device_manager_create(state->display);
    wlr_data_control_manager_v1_create(state->display);
    wlr_xdg_output_manager_v1_create(state->display, state->output_layout);
    wlr_viewporter_create(state->display);
    wlr_presentation_create(state->display, state->backend.wlr_backend, COMPHY_PRESENTATION_VERSION);
    wlr_screencopy_manager_v1_create(state->display);
    wlr_export_dmabuf_manager_v1_create(state->display);
    wlr_fractional_scale_manager_v1_create(state->display, COMPHY_FRACTIONAL_SCALE_VERSION);
    // TODO: wire up these so they actually do something
    wlr_virtual_pointer_manager_v1_create(state->display);
    wlr_virtual_keyboard_manager_v1_create(state->display);

    // TODO: rewire this in the future
    // server.relative_pointer_manager = wlr_relative_pointer_manager_v1_create(server.wl_display);
    // server.relative_pointer_manager_destroy.notify = server_handle_relative_pointer_manager_destroy;
    // wl_signal_add(&server.relative_pointer_manager->events.destroy, &server.relative_pointer_manager_destroy);
    //
    // server.pointer_contrains_manager = wlr_pointer_constraints_v1_create(server.wl_display);
    // server.new_contraint.notify = server_handle_new_constraint;
    // wl_signal_add(&server.pointer_contrains_manager->events.new_constraint, &server.new_contraint);

    const char *socket = wl_display_add_socket_auto(state->display);
    if(!socket) {
        wlr_log(WLR_ERROR, "could not create the socket");
        goto cleanup;
    }

    if(!backend_start(&state->backend)) {
        wlr_log(WLR_ERROR, "could not start the backend");
        goto cleanup;
    }

    setenv("WAYLAND_DISPLAY", socket, true);
    wlr_log(WLR_INFO, "running 'comphy' on socket '%s'", socket);
    wl_display_run(state->display);

    // destroy all clients and quit
    wl_display_destroy_clients(state->display);
cleanup:
    scene_deinit(&state->scene);
    seat_deinit(&state->seat);
    xdg_shell_deinit(&state->xdg_shell);
    layer_shell_deinit(&state->layer_shell);
    cursor_deinit(&state->cursor);
    decoration_deinit(&state->decoration);
    gamma_control_deinit(&state->gamma_control);
    backend_deinit(&state->backend);
config:
    config_deinit(&state->config);
    wl_display_destroy(state->display);
err:
    return 0;
}
