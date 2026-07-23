#ifndef DND_H
#define DND_H

#include <stdbool.h>
#include <wayland-server.h>

void
dnd_init(void);

void
dnd_icons_show(bool show);

void
dnd_icons_move(uint32_t x, uint32_t y);

#endif
