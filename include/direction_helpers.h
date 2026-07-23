#ifndef DIRECTION_HELPERS_H
#define DIRECTION_HELPERS_H

#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/box.h>

static inline enum wlr_direction
direction_opposite(enum wlr_direction direction) {
    switch(direction) {
        case WLR_DIRECTION_UP: {
            return WLR_DIRECTION_DOWN;
        }
        case WLR_DIRECTION_RIGHT: {
            return WLR_DIRECTION_LEFT;
        }
        case WLR_DIRECTION_DOWN: {
            return WLR_DIRECTION_UP;
        }
        case WLR_DIRECTION_LEFT: {
            return WLR_DIRECTION_RIGHT;
        }
    }
}

#endif
