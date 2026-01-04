#include "comphy.h"

#include <stdbool.h>
#include <stdio.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/util/log.h>

struct g g = {0};

static bool
init_backend() {
    return true;
}

int
main(int argc, char **argv) {
    printf("Hello, World!\n");
    enum wlr_log_importance log_level = WLR_INFO;

    int ret = 0;

    // the wayland display is managed by libwayland. it handles accepting clients from the unix socket, manging
    // wayland globals, and so on
    g.display = wl_display_create();
    if(!g.display) {
        wlr_log(WLR_ERROR, "failed to create the display");
        ret = 1;
        goto err;
    }

    g.backend = backend_create(g.display);
    if(!g.backend) {
        wlr_log(WLR_ERROR, "failed to create the backend");
        ret = 1;
        goto err;
    }

    g.renderer = wlr_renderer_autocreate(g.backend);
    if(!g.renderer) {
        wlr_log(WLR_ERROR, "failed to create the renderer");
        ret = 1;
        goto backend;
    }

    wlr_renderer_init_wl_display(g.renderer, g.display);

    g.allocator = wlr_allocator_autocreate(g.backend, g.renderer);
    if(!g.allocator) {
        wlr_log(WLR_ERROR, "failed to create the allocator");
        ret = 1;
        goto renderer;
    }

    // create all the systems and attach listeners
    wlr_compositor_create(g.display, 6, g.renderer);
    wlr_subcompositor_create(g.display);

    g.output_layout = wlr_output_layout_create(g.display);

    g.scene = wlr_scene_create();

    // autocreates an allocator for us. the allocator is the bridge between the renderer and the backend. it handles
    // the buffer creation, allowing wlroots to render onto the screen
    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if(server.allocator == NULL) {
        wlr_log(WLR_ERROR, "failed to create the allocator");
        ret = 1;
        goto renderer;
    }

    // create all the systems and attach listeners
    wlr_compositor_create(server.display, 6, server.renderer);
    wlr_subcompositor_create(server.display);

    // outputs
    wl_list_init(&server.outputs);

    server.new_output.notify = handle_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    server.output_layout = wlr_output_layout_create(server.display);

    // create a scene graph. this is a wlroots abstraction that handles all rendering and damage tracking. all the
    // compositor author needs to do is add things that should be rendered to the scene graph at the proper
    // positions and then call wlr_scene_output_commit() to render a frame
    server.scene = wlr_scene_create();
    server.scene_layout = wlr_scene_attach_output_layout(server.scene, server.output_layout);

    // create all the scene trees in the correct order
    server.background_tree = wlr_scene_tree_create(&server.scene->tree);
    server.bottom_tree = wlr_scene_tree_create(&server.scene->tree);
    server.tiled_tree = wlr_scene_tree_create(&server.scene->tree);
    server.floating_tree = wlr_scene_tree_create(&server.scene->tree);
    server.top_tree = wlr_scene_tree_create(&server.scene->tree);
    server.fullscreen_tree = wlr_scene_tree_create(&server.scene->tree);
    server.grabbed_tree = wlr_scene_tree_create(&server.scene->tree);
    server.overlay_tree = wlr_scene_tree_create(&server.scene->tree);
    server.session_lock_tree = wlr_scene_tree_create(&server.scene->tree);
    server.drag_icon_tree = wlr_scene_tree_create(&server.scene->tree);

    // set the initial blur params
    wlr_scene_set_blur_data(server.scene, server.config->blur.params);

    font_manager_init();
    if(server.config->titlebar.title.fonts != NULL) {
        server.title_font = font_create(array_len(server.config->titlebar.title.fonts),
                server.config->titlebar.title.fonts, server.config->titlebar.title.size);
    }

    seat_init();
    cursor_init();

    xdg_shell_init();
    layer_shell_init();

    wlr_data_device_manager_create(server.display);
    wlr_data_control_manager_v1_create(server.display);
    wlr_fractional_scale_manager_v1_create(server.display, 1);
    wlr_xdg_output_manager_v1_create(server.display, server.output_layout);
    wlr_viewporter_create(server.display);
    wlr_presentation_create(server.display, server.backend);
    wlr_screencopy_manager_v1_create(server.display);
    wlr_export_dmabuf_manager_v1_create(server.display);

    server.foreign_toplevel_manager = wlr_foreign_toplevel_manager_v1_create(server.display);

    gamma_manager_init();
    lock_manager_init();
    relative_pointer_manager_init();
    constraint_manager_init();

    // todo: these need some wiring i think
    wlr_virtual_pointer_manager_v1_create(server.display);
    wlr_virtual_keyboard_manager_v1_create(server.display);

    fx_animation_manager_init(server.display, server.scene);
    server.animation_curve = fx_animation_curve_create(server.config->animations.curve);

    // Add a unix socket to the wayland display
    const char *socket = wl_display_add_socket_auto(server.display);
    if(!socket) {
        wlr_log(WLR_ERROR, "failed to add a socket");
        ret = 1;
        goto scene;
    }

    // start the backend
    if(!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "failed to start the backend");
        ret = 1;
        goto scene;
    }

    // set the WAYLAND_DISPLAY environment variable to our socket
    setenv("WAYLAND_DISPLAY", socket, true);

    // add other relevant fds and signal handlers to the main loop
    struct wl_event_source *sigchld_source =
            wl_event_loop_add_signal(server.event_loop, SIGCHLD, sigchld_handler, NULL);
    struct wl_event_source *sigpipe_source =
            wl_event_loop_add_signal(server.event_loop, SIGPIPE, sigpipe_handler, NULL);

    ipc_init();

    if(server.config_path != NULL) {
        char *config_dir = get_parent_dir_path(server.config_path);
        config_watcher_init(config_dir);
        string_destroy(config_dir);
    }

    // run the startup commands
    for(size_t i = 0; i < array_len(server.config->on_startup); i++) {
        run_cmd(server.config->on_startup[i]);
    }

    // run the wayland event loop
    wlr_log(WLR_INFO, "running mwc on WAYLAND_DISPLAY=%s", socket);
    wl_display_run(server.display);

    // after the loop returns cleanup
    if(ipc_running()) {
        ipc_deinit();
    }

    if(config_watcher_running()) {
        config_watcher_deinit();
    }

    font_manager_deinit();

    wl_event_source_remove(sigchld_source);
    wl_event_source_remove(sigpipe_source);

    // destroy server resources
    wl_display_destroy_clients(server.display);
scene:
    wlr_scene_node_destroy(&server.scene->tree.node);
    wlr_xcursor_manager_destroy(server.cursor.theme_manager);
    wlr_cursor_destroy(server.cursor.base);
    wlr_allocator_destroy(server.allocator);
renderer:
    wlr_renderer_destroy(server.renderer);
backend:
    wlr_backend_destroy(server.backend);
display:
    wl_display_destroy(server.display);
config:
    config_destroy(server.config);
config_path:
    if(server.config_path != NULL) {
        string_destroy(server.config_path);
    }

    return ret;
}
