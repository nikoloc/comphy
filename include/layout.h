#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdint.h>

#include "workspace.h"

void
layout_add(struct workspace *workspace, struct toplevel *toplevel);

void
layout_remove(struct toplevel *toplevel);

void
layout_configure(struct state *state, struct workspace *workspace);

void
layout_reconfigure_all(struct state *state);

#endif
