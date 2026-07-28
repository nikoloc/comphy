#include "ctl.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

#include "comphy.h"
#include "state.h"
#include "util/hashmap.h"
#include "util/macros.h"

// we use a hashmap for saving options for a given command
DEFINE_HASHMAP(string_t, string_hashmap);

static int
handle_sigpipe(int signal_number, void *data) {
    UNUSED(signal_number), UNUSED(data);

    // do nothing
    return 0;
}

static int
handle_command(int fd, uint32_t mask, void *data) {
    UNUSED(data);

    struct state *state = state_get();

    if((mask & WL_EVENT_ERROR) || (mask & WL_EVENT_HANGUP)) {
        wlr_log(WLR_ERROR, "error occurred on the socket");
        ctl_deinit(&state->ctl);
        return 0;
    }

    struct sockaddr_un client_address;
    socklen_t sock_len;
    int client_fd = accept(fd, (struct sockaddr *)&client_address, &sock_len);
    if(client_fd < 0) {
        wlr_log(WLR_ERROR, "failed to accept client");
        return 0;
    }

    char buffer[1024];
    ssize_t len = read(client_fd, buffer, sizeof(buffer) - 1);
    if(len < 0) {
        wlr_log(WLR_ERROR, "failed read on fd '%d'", client_fd);
        close(client_fd);
        return 0;
    }

    buffer[len] = 0;
    handle_request(client_fd, buffer);

    return 0;
}

void
ctl_init(struct ctl *ctl, struct wl_display *display) {
    ctl->fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(ctl->fd < 0) {
        wlr_log(WLR_ERROR, "failed to open a socket: %s", strerror(errno));
        return;
    }

    struct sockaddr_un address = {0};
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, COMPHYCTL_SOCKET);

    if(bind(ctl->fd, (struct sockaddr *)&address, sizeof(address))) {
        wlr_log(WLR_ERROR, "failed to bind to socket: %s", strerror(errno));
        close(ctl->fd);
        return;
    }

    if(listen(ctl->fd, 128) < 0) {
        wlr_log(WLR_ERROR, "ipc: failed to listen on socket: %s", strerror(errno));
        close(ctl->fd);
        unlink(COMPHYCTL_SOCKET);
        return;
    }

    struct wl_event_loop *event_loop = wl_display_get_event_loop(display);
    ctl->source = wl_event_loop_add_fd(event_loop, ctl->fd, WL_EVENT_READABLE | WL_EVENT_HANGUP | WL_EVENT_ERROR,
            handle_command, NULL);
}

void
ipc_deinit(struct ctl *ctl) {
    if(ctl->has_exited) {
        // has already exited, due to a problem, so no cleanup required
        return;
    }

    ctl->has_exited = true;
    wl_event_source_remove(ctl->source);

    close(ctl->fd);
    unlink(COMPHYCTL_SOCKET);
}

static void
handle_get(int fd, char *what) {
    if(strcmp(what, "focused_toplevel") == 0) {
        if(server.focused_toplevel == NULL) {
            write(fd, "null", strlen("null"));
            return;
        }

        struct json_object *toplevel = toplevel_json(server.focused_toplevel);

        size_t len;
        const char *stringified = json_object_to_json_string_length(toplevel, JSON_C_TO_STRING_PRETTY, &len);

        write(fd, stringified, len);

        json_object_put(toplevel);
    } else if(strcmp(what, "focused_layer") == 0) {
        if(server.focused_layer_surface == NULL) {
            write(fd, "null", strlen("null"));
            return;
        }

        struct json_object *layer = layer_json(server.focused_layer_surface);

        size_t len;
        const char *stringified = json_object_to_json_string_length(layer, JSON_C_TO_STRING_PRETTY, &len);

        write(fd, stringified, len);

        json_object_put(layer);
    } else if(strcmp(what, "active_workspace") == 0) {
        struct json_object *workspace = workspace_json(server.active_workspace);

        size_t len;
        const char *stringified = json_object_to_json_string_length(workspace, JSON_C_TO_STRING_PRETTY, &len);

        write(fd, stringified, len);

        json_object_put(workspace);
    } else if(strcmp(what, "toplevels") == 0) {
        struct json_object *array = json_object_new_array();

        struct output *iter_output;
        wl_list_for_each(iter_output, &server.outputs, link) {
            struct workspace *iter_workspace;
            wl_list_for_each(iter_workspace, &iter_output->workspaces, link) {
                struct toplevel *iter_toplevel;
                wl_list_for_each(iter_toplevel, &iter_workspace->floating, link) {
                    json_object_array_add(array, toplevel_json(iter_toplevel));
                }
                wl_list_for_each(iter_toplevel, &iter_workspace->masters, link) {
                    json_object_array_add(array, toplevel_json(iter_toplevel));
                }
                wl_list_for_each(iter_toplevel, &iter_workspace->slaves, link) {
                    json_object_array_add(array, toplevel_json(iter_toplevel));
                }

                if(iter_workspace->fullscreen != NULL) {
                    json_object_array_add(array, toplevel_json(iter_workspace->fullscreen));
                }
            }
        }

        if(server.grabbed_toplevel != NULL) {
            json_object_array_add(array, toplevel_json(server.grabbed_toplevel));
        }

        size_t len;
        const char *stringified = json_object_to_json_string_length(array, JSON_C_TO_STRING_PRETTY, &len);

        write(fd, stringified, len);

        json_object_put(array);
    } else if(strcmp(what, "layers") == 0) {
        struct json_object *array = json_object_new_array();

        struct output *iter_output;
        wl_list_for_each(iter_output, &server.outputs, link) {
            struct layer_surface *iter_layer;
            for(size_t i = 0; i < 4; i++) {
                wl_list_for_each(iter_layer, &(&iter_output->layers.background)[i], link) {
                    json_object_array_add(array, layer_json(iter_layer));
                }
            }
        }

        size_t len;
        const char *stringified = json_object_to_json_string_length(array, JSON_C_TO_STRING_PRETTY, &len);

        write(fd, stringified, len);

        json_object_put(array);
    } else if(strcmp(what, "workspaces") == 0) {
        struct json_object *array = json_object_new_array();

        struct output *iter_output;
        wl_list_for_each(iter_output, &server.outputs, link) {
            struct workspace *iter_workspace;
            wl_list_for_each(iter_workspace, &iter_output->workspaces, link) {
                json_object_array_add(array, workspace_json(iter_workspace));
            }
        }

        size_t len;
        const char *stringified = json_object_to_json_string_length(array, JSON_C_TO_STRING_PRETTY, &len);

        write(fd, stringified, len);

        json_object_put(array);
    } else if(strcmp(what, "outputs") == 0) {
        struct json_object *array = json_object_new_array();

        struct output *iter_output;
        wl_list_for_each(iter_output, &server.outputs, link) {
            json_object_array_add(array, output_json(iter_output));
        }

        size_t len;
        const char *stringified = json_object_to_json_string_length(array, JSON_C_TO_STRING_PRETTY, &len);

        write(fd, stringified, len);

        json_object_put(array);
    } else {
        write(fd, "null", strlen("null"));
    }

    close(fd);
}

static void
handle_watch(int fd, char *what) {
    if(strcmp(what, "active_workspace") == 0) {
        array_push(&server.ipc.watching_workspace, fd);
        ipc_send_active_workspace();
    } else if(strcmp(what, "focused_toplevel") == 0) {
        array_push(&server.ipc.watching_toplevel, fd);
        ipc_send_focused_toplevel();
    } else if(strcmp(what, "focused_layer") == 0) {
        array_push(&server.ipc.watching_layer, fd);
        ipc_send_focused_layer();
    } else {
        write(fd, "null", strlen("null"));
        close(fd);
    }
}

static void
handle_action(int fd, char *action, char **args, size_t arg_count) {
    if(strcmp(action, "exit") == 0) {
        keybind_stop_server(NULL);
    } else if(strcmp(action, "run") == 0) {
        if(arg_count < 1)
            goto done;

        keybind_run(args[0]);
    } else if(strcmp(action, "close") == 0) {
        keybind_close(NULL);
    } else if(strcmp(action, "toggle_floating") == 0) {
        keybind_toggle_floating(NULL);
    } else if(strcmp(action, "move_focus") == 0) {
        if(arg_count < 1)
            goto done;

        enum direction direction;
        if(strcmp(args[0], "up") == 0) {
            direction = DIRECTION_UP;
        } else if(strcmp(args[0], "left") == 0) {
            direction = DIRECTION_LEFT;
        } else if(strcmp(args[0], "down") == 0) {
            direction = DIRECTION_DOWN;
        } else if(strcmp(args[0], "right") == 0) {
            direction = DIRECTION_RIGHT;
        } else {
            goto done;
        }

        keybind_move_focus((void *)direction);
    } else if(strcmp(action, "move") == 0) {
        if(arg_count < 1)
            goto done;

        enum direction direction;
        if(strcmp(args[0], "up") == 0) {
            direction = DIRECTION_UP;
        } else if(strcmp(args[0], "left") == 0) {
            direction = DIRECTION_LEFT;
        } else if(strcmp(args[0], "down") == 0) {
            direction = DIRECTION_DOWN;
        } else if(strcmp(args[0], "right") == 0) {
            direction = DIRECTION_RIGHT;
        } else {
            goto done;
        }

        keybind_move((void *)direction);
    } else if(strcmp(action, "workspace") == 0) {
        if(arg_count < 1)
            goto done;

        keybind_change_workspace((void *)(intptr_t)atoi(args[0]));
    } else if(strcmp(action, "move_to_workspace") == 0) {
        if(arg_count < 1)
            goto done;

        keybind_move_to_workspace((void *)(intptr_t)atoi(args[0]));
    } else if(strcmp(action, "next_workspace") == 0) {
        keybind_next_workspace(NULL);
    } else if(strcmp(action, "prev_workspace") == 0) {
        keybind_prev_workspace(NULL);
    } else if(strcmp(action, "toggle_fullscreen") == 0) {
        keybind_toggle_fullscreen(NULL);
    } else if(strcmp(action, "toggle_fake_fullscreen") == 0) {
        keybind_toggle_fake_fullscreen(NULL);
    } else if(strcmp(action, "master_ratio") == 0) {
        if(arg_count < 1)
            goto done;

        if(args[0][1] == '+') {
            keybind_adjust_master_ratio((void *)(intptr_t)(atof(&args[0][1]) * 10000));
        } else if(args[0][1] == '-') {
            keybind_adjust_master_ratio((void *)(intptr_t)(-atof(&args[0][1]) * 10000));
        } else {
            keybind_set_master_ratio((void *)(intptr_t)(atof(args[0]) * 10000));
        }
    }

done:
    close(fd);
}

static void
handle_request(int fd, char *buffer) {
    char **words = parser_into_words(buffer, 0);

    if(array_len(words) < 2) {
        close(fd);
    } else if(strcmp(words[0], "get") == 0) {
        handle_get(fd, words[1]);
    } else if(strcmp(words[0], "watch") == 0) {
        handle_watch(fd, words[1]);
    } else if(strcmp(words[0], "action") == 0) {
        handle_action(fd, words[1], &words[2], array_len(words) - 2);
    } else {
        close(fd);
    }

    array_destroy(words);
}
