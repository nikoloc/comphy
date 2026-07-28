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
#define ARGUMENTS_PARSER_IMPLEMENTATION
#include "util/arguments_parser.h"
#include "util/macros.h"

static int
handle_sigpipe(int signal_number, void *data) {
    UNUSED(signal_number), UNUSED(data);

    // do nothing
    return 0;
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

static inline bool
string_starts_with(char *str, char *prefix) {
    while(*prefix) {
        if(*str != *prefix) {
            return false;
        }

        str++;
        prefix++;
    }

    return true;
}

static void
handle_request(int fd, char *buffer) {
    if(string_starts_with(buffer, "create_workspace")) {
    } else if(string_starts_with(buffer, "change_workspace")) {
    } else if(string_starts_with(buffer, "move_to_workspace")) {
    } else if(string_starts_with(buffer, "focus")) {
    } else if(string_starts_with(buffer, "move")) {
    } else if(string_starts_with(buffer, "exec")) {
    } else if(string_starts_with(buffer, "env")) {
    } else if(string_starts_with(buffer, "keyboard")) {
    } else if(string_starts_with(buffer, "pointer")) {
    } else if(string_starts_with(buffer, "trackpad")) {
    } else if(string_starts_with(buffer, "cursor")) {
    } else if(string_starts_with(buffer, "gaps")) {
    } else if(string_starts_with(buffer, "border")) {
    } else if(string_starts_with(buffer, "toplevel")) {
    } else if(string_starts_with(buffer, "exit")) {
    } else if(string_starts_with(buffer, "toggle_floating")) {
    } else if(string_starts_with(buffer, "toggle_fullscreen")) {
    } else if(string_starts_with(buffer, "toggle_fake_fullscreen")) {
    } else if(string_starts_with(buffer, "start_move")) {
    } else if(string_starts_with(buffer, "start_resize")) {
    } else if(string_starts_with(buffer, "adjust_master_ratio")) {
    } else if(string_starts_with(buffer, "create_keybind")) {
    }
}

static int
handle_command(int fd, uint32_t mask, void *data) {
    UNUSED(data);

    struct state *state = state_get();

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
    ctl->source = wl_event_loop_add_fd(event_loop, ctl->fd, WL_EVENT_READABLE, handle_command, NULL);
}

void
ipc_deinit(struct ctl *ctl) {
    wl_event_source_remove(ctl->source);

    close(ctl->fd);
    unlink(COMPHYCTL_SOCKET);
}
