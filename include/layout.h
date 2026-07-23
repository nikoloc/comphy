#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdint.h>

#include "comphy.h"
#include "output.h"

void
layout_configure(struct workspace *workspace);

// t2 needs to be after t1 if in the same list (both slaves)
void
layout_swap(struct toplevel *t1, struct toplevel *t2);

struct toplevel *
layout_toplevel_at(struct workspace *workspace, int x, int y);

#endif
