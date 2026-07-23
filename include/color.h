#ifndef COLOR_H
#define COLOR_H

#include <stdbool.h>

#include "pixman.h"
#include "util/ints.h"

typedef struct color {
    u8 r, g, b, a;
} color_t;

bool
color_from_hex(char *s, color_t *dest);

void
color_to_wlr_color(color_t color, float dest[static 4]);

void
color_to_pixman_color(color_t color, pixman_color_t *dest);

void
color_premultiply(struct color *color);

#endif
