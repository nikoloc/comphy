#ifndef VIEW_H
#define VIEW_H

#include <wlr/types/wlr_compositor.h>

enum view {
    VIEW_TOPLEVEL,
    VIEW_POPUP,
    VIEW_LAYER,
    VIEW_LOCK_SURFACE,
};

enum view *
view_root_parent_of_surface(struct wlr_surface *surface);

struct state;

enum view *
view_at(struct state *state, double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);

// get the focused views in the correct focus rule order: lock, layer, toplevel
enum view *
view_get_focused(struct state *state);

struct output *
view_get_output(enum view *view);

void
view_focus(struct state *state, enum view *view);

#endif
