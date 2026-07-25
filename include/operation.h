#ifndef OPERATION_H
#define OPERATION_H

#include "toplevel.h"
#include "util/ints.h"

enum operation {
    OPERATION_NONE = 0,
    OPERATION_MOVE,
    OPERATION_RESIZE,
    OPERATION_DRAG,
};

struct state;

void
operation_start_move(struct state *state, struct toplevel *toplevel);

void
operation_stop_move(struct state *state);

void
operation_start_resize(struct state *state, struct toplevel *toplevel, u32 edges);

void
operation_stop_resize(struct state *state);

// TODO: add whatever we need here
void
operation_start_drag(struct state *state);

void
operation_stop_drag(struct state *state);

void
operation_stop_whatever(struct state *state);

#endif
