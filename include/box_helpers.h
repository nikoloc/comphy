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

static inline void
wlr_box_remove_gaps(struct wlr_box *box, int gaps) {
    box->x += gaps;
    box->y += gaps;
    box->width -= 2 * gaps;
    box->height -= 2 * gaps;
}

static inline void
wlr_box_add_gaps(struct wlr_box *box, int gaps) {
    box->x -= gaps;
    box->y -= gaps;
    box->width += 2 * gaps;
    box->height += 2 * gaps;
}

static inline struct wlr_box
wlr_box_centered_in(struct wlr_box *box, int width, int height) {
    return (struct wlr_box){
            .x = box->x + (box->width - width) / 2.0f,
            .y = box->y + (box->height - height) / 2.0f,
            .width = width,
            .height = height,
    };
}

#endif
