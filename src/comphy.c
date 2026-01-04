#include "comphy.h"

#include <stdbool.h>
#include <stdlib.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/util/log.h>

#include "macros.h"

struct g g = {0};

#define goto_error(label) \
    ret = 1;              \
    goto label

int
main(int argc, char **argv) {
    unused(argc), unused(argv);

    int ret = 0;

    enum wlr_log_importance log_level = WLR_INFO;
    wlr_log_init(log_level, NULL);

    g.display = wl_display_create();
    if(!g.display) {
        wlr_log(WLR_ERROR, "failed to create the display");
        goto_error(err);
    }

    g.backend = backend_create(g.display);
    if(!g.backend) {
        wlr_log(WLR_ERROR, "failed to create the backend");
        goto_error(err);
    }

    wlr_compositor_create(g.display, 6, g.renderer);
    wlr_subcompositor_create(g.display);

    g.scene = wlr_scene_create();

    // server.output_layout = wlr_output_layout_create(server.display);

    // server.scene = wlr_scene_create();
    // server.scene_layout = wlr_scene_attach_output_layout(server.scene, server.output_layout);

    // create all the scene trees in the correct order
    // server.background_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.bottom_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.tiled_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.floating_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.top_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.fullscreen_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.grabbed_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.overlay_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.session_lock_tree = wlr_scene_tree_create(&server.scene->tree);
    // server.drag_icon_tree = wlr_scene_tree_create(&server.scene->tree);

    // font_manager_init();
    // if(server.config->titlebar.title.fonts != NULL) {
    //     server.title_font = font_create(array_len(server.config->titlebar.title.fonts),
    //             server.config->titlebar.title.fonts, server.config->titlebar.title.size);
    // }

    // seat_init();
    // cursor_init();
    //
    // xdg_shell_init();
    // layer_shell_init();
    //
    // wlr_data_device_manager_create(server.display);
    // wlr_data_control_manager_v1_create(server.display);
    // wlr_fractional_scale_manager_v1_create(server.display, 1);
    // wlr_xdg_output_manager_v1_create(server.display, server.output_layout);
    // wlr_viewporter_create(server.display);
    // wlr_presentation_create(server.display, server.backend);
    // wlr_screencopy_manager_v1_create(server.display);
    // wlr_export_dmabuf_manager_v1_create(server.display);
    //
    // server.foreign_toplevel_manager = wlr_foreign_toplevel_manager_v1_create(server.display);

    // gamma_manager_init();
    // lock_manager_init();
    // relative_pointer_manager_init();
    // constraint_manager_init();
    //
    // wlr_virtual_pointer_manager_v1_create(server.display);
    // wlr_virtual_keyboard_manager_v1_create(server.display);
    //
    // fx_animation_manager_init(server.display, server.scene);
    // server.animation_curve = fx_animation_curve_create(server.config->animations.curve);

    const char *socket = wl_display_add_socket_auto(g.display);
    if(!socket) {
        wlr_log(WLR_ERROR, "failed to add a socket");
        GOTO_ERROR(scene);
    }

    if(!backend_start(g.backend)) {
        wlr_log(WLR_ERROR, "failed to start the backend");
        GOTO_ERROR(scene);
    }

    setenv("WAYLAND_DISPLAY", socket, true);

    // add other relevant fds and signal handlers to the main loop
    // struct wl_event_source *sigchld_source =
    //         wl_event_loop_add_signal(server.event_loop, SIGCHLD, sigchld_handler, NULL);
    // struct wl_event_source *sigpipe_source =
    //         wl_event_loop_add_signal(server.event_loop, SIGPIPE, sigpipe_handler, NULL);

    // for(size_t i = 0; i < array_len(server.config->on_startup); i++) {
    //     run_cmd(server.config->on_startup[i]);
    // }

    // run the wayland event loop
    wlr_log(WLR_INFO, "running comphy on socket `%s`", socket);
    wl_display_run(g.display);

    // font_manager_deinit();

    // destroy server resources
    wl_display_destroy_clients(g.display);
scene:
    // wlr_scene_node_destroy(&.scene->tree.node);
    // wlr_xcursor_manager_destroy(server.cursor.theme_manager);
    // wlr_cursor_destroy(server.cursor.base);
backend:
    backend_destroy(g.backend);
display:
    wl_display_destroy(g.display);
err:
    return ret;
}
