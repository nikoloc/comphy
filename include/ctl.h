#ifndef CTL_H
#define CTL_H

#include <wayland-server-core.h>

struct ctl {
    bool has_exited;

    int fd;
    struct wl_event_source *source;
};

void
ctl_init(struct ctl *ctl, struct wl_display *display);

void
ctl_deinit(struct ctl *ctl);

#endif
