#include "ctl.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <wlr/util/log.h>

#include "comphy.h"
#include "state.h"
#include "util/macros.h"
#include "util/shell_parser.h"

static void
handle_request(struct state *state, int fd, char *buffer) {
    UNUSED(fd);

    struct shell_parser parser;
    shell_parser_init(&parser, buffer);

    enum action_type type;
    void *action = NULL;

    if(!action_create(&parser, &type, &action)) {
        wlr_log(WLR_ERROR, "invalid request");
        return;
    }

    action_perform(state, type, action);
    action_destroy(type, action);
}

static int
handle_command(int fd, u32 mask, void *data) {
    UNUSED(data), UNUSED(mask);

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

    wlr_log(WLR_DEBUG, "new ctl cmd: %s", buffer);

    buffer[len] = 0;
    handle_request(state, client_fd, buffer);

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
ctl_deinit(struct ctl *ctl) {
    if(ctl->source) {
        wl_event_source_remove(ctl->source);
    }

    if(ctl->fd > 0) {
        close(ctl->fd);
    }
    unlink(COMPHYCTL_SOCKET);
}
