#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "toplevel.h"

void
transaction_commit(struct toplevel *toplevel);

void
transaction_schedule_commit(struct state *state, struct toplevel *toplevel);

void
transaction_mark_dirty(struct toplevel *toplevel);

#endif
