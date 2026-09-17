#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <wlr/util/box.h>

enum transaction_state {
    TRANSACTION_STATE_CLEAN = 0,
    TRANSACTION_STATE_DIRTY,
    TRANSACTION_STATE_READY,
};

struct toplevel;
struct state;

void
transaction_add_dirty(struct state *state, struct toplevel *toplevel, struct wlr_box *box);

void
transaction_add_auto(struct state *state, struct toplevel *toplevel);

void
transaction_add_ghost(struct state *state, struct toplevel *toplevel);

void
transaction_add_ready(struct state *state, struct toplevel *toplevel);

void
transaction_schedule(struct state *state);

void
transaction_commit(struct state *state, struct toplevel *toplevel);

#endif
