#ifndef BOX_HELPERS_H
#define BOX_HELPERS_H

#include <wlr/util/box.h>

static inline int
wlr_box_area(struct wlr_box *box) {
    return box->width * box->height;
}

static inline void
wlr_box_midpoint(const struct wlr_box *box, int *x, int *y) {
    *x = box->x + box->width / 2;
    *y = box->y + box->height / 2;
}

static inline void
wlr_box_same_relative_position(struct wlr_box *old, struct wlr_box *new, int *x, int *y) {
    double relative_x = (double)(*x - old->x) / old->width;
    double relative_y = (double)(*y - old->y) / old->height;

    *x = new->x + relative_x *new->width;
    *y = new->y + relative_y *new->height;
}

#endif
