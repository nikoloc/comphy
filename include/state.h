#ifndef STATE_H
#define STATE_H

#include "backend.h"
#include "config.h"
#include "ctl.h"
#include "cursor.h"
#include "decoration.h"
#include "gamma_control.h"
#include "layer_shell.h"
#include "lock.h"
#include "operation.h"
#include "scene.h"
#include "seat.h"
#include "util/ints.h"
#include "xdg_shell.h"

// piece of global compositor state, passed to all the 'public' apis. since it is quite hard to get the to it from the
// callbacks, it is made a singleton struct, which can be obtained in the callback context by calling `state()`. note
// that this function should not be called from elsewhere
struct state {
    struct wl_display *display;
    bool is_exiting;

    struct wlr_output_layout *output_layout;

    struct backend backend;
    struct scene scene;
    struct ctl ctl;

    // interfaces we keep track of
    struct xdg_shell xdg_shell;
    struct layer_shell layer_shell;
    struct seat seat;
    struct decoration decoration;
    struct cursor cursor;
    struct lock_mgr lock_mgr;
    struct gamma_control gamma_control;
    struct wlr_foreign_toplevel_manager_v1 *foreign_toplevel_manager;

    struct wl_list pointers;
    struct wl_list keyboards;
    struct wl_list outputs;

    struct wl_list keybinds;

    enum operation operation;
    struct toplevel *grabbed_toplevel;
    double grab_x, grab_y;
    struct wlr_box grabbed_toplevel_initial_box;
    u32 resize_edges;
    bool client_driven_move_resize;

    struct workspace *active_workspace;

    struct lock_surface *focused_lock;
    struct layer *focused_layer;
    bool is_exclusive;
    struct toplevel *prev_focused;
    struct toplevel *focused_toplevel;

    struct wlr_pointer_constraints_v1 *pointer_contrains_manager;
    struct wl_listener new_contraint;
    struct pointer_constraint *current_constraint;

    struct wlr_relative_pointer_manager_v1 *relative_pointer_manager;
    struct wl_listener relative_pointer_manager_destroy;

    struct config config;
};

struct state *
state_get(void);

#endif
