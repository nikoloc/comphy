#ifndef TRANSACTION_H
#define TRANSACTION_H

enum transaction_state {
    TRANSACTION_STATE_CLEAN = 0,
    TRANSACTION_STATE_DIRTY,
    TRANSACTION_STATE_READY,
};

struct toplevel;
struct state;
struct workspace;

void
transaction_commit(struct state *state, struct toplevel *toplevel);

void
transaction_schedule_commit(struct state *state, struct workspace *workspace);

void
transaction_mark_dirty(struct state *state, struct toplevel *toplevel);

#endif
