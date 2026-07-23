#include "color.h"

#include <stdio.h>

bool
color_from_hex(char *s, color_t *dest) {
    unsigned int r, g, b, a;

    int found = sscanf(s, "#%02x%02x%02x%02x", &r, &g, &b, &a);

    if(found == 4) {
        dest->r = r;
        dest->g = g;
        dest->b = b;
        dest->a = a;

        return true;
    } else if(found == 3) {
        dest->r = r;
        dest->g = g;
        dest->b = b;
        dest->a = UINT8_MAX;

        return true;
    } else {
        return false;
    }
}

void
color_to_wlr_color(color_t color, float dest[static 4]) {
    dest[0] = color.r / 255.0;
    dest[1] = color.g / 255.0;
    dest[2] = color.b / 255.0;
    dest[3] = color.a / 255.0;
}

void
color_to_pixman_color(color_t color, pixman_color_t *dest) {
    dest->red = color.r * 257;
    dest->green = color.g * 257;
    dest->blue = color.b * 257;
    dest->alpha = color.a * 257;
}

void
color_premultiply(struct color *color) {
    color->r *= color->a / 255.0;
    color->g *= color->a / 255.0;
    color->b *= color->a / 255.0;
}
