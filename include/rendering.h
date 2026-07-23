#ifndef RENDERING_H
#define RENDERING_H

#include <stdint.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/box.h>

#include "toplevel.h"
#include "workspace.h"

enum border_state {
    BORDER_STATE_INVISIBLE,
    BORDER_STATE_ACTIVE,
    BORDER_STATE_INACTIVE,
};

void
toplevel_draw_borders(struct toplevel *toplevel);

bool
toplevel_draw_frame(struct toplevel *toplevel);

void
toplevel_apply_clip(struct toplevel *toplevel);

void
toplevel_unclip_size(struct toplevel *toplevel);

void
workspace_draw_frame(struct workspace *workspace);

void
toplevel_apply_effects(struct toplevel *toplevel);

#endif
