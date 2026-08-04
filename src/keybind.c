#include "keybind.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wayland-util.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/xcursor.h>

#include "config.h"
#include "layout.h"
#include "mwc.h"
#include "toplevel.h"
#include "workspace.h"

// void
// keybind_move_focus(void *data) {
//     uint64_t direction = (uint64_t)data;
//
//     struct mwc_toplevel *toplevel = server.focused_toplevel;
//     /* we need grabbed toplevel toplevel to keep focus */
//     if(server.grabbed_toplevel != NULL && toplevel == server.grabbed_toplevel)
//         return;
//
//     enum mwc_direction opposite_side;
//     switch(direction) {
//         case MWC_UP:
//             opposite_side = MWC_DOWN;
//             break;
//         case MWC_DOWN:
//             opposite_side = MWC_UP;
//             break;
//         case MWC_LEFT:
//             opposite_side = MWC_RIGHT;
//             break;
//         case MWC_RIGHT:
//             opposite_side = MWC_LEFT;
//             break;
//     }
//
//     /* if no toplevel has keyboard focus then get the output
//      * the pointer is on and try from there */
//     if(toplevel == NULL) {
//         struct wlr_output *wlr_output =
//                 wlr_output_layout_output_at(server.output_layout, server.cursor->x, server.cursor->y);
//         struct mwc_output *output = wlr_output->data;
//         struct mwc_output *relative_output = output_get_relative(output, direction);
//         if(relative_output != NULL) {
//             focus_output(relative_output, opposite_side);
//         }
//         return;
//     }
//
//     /* get the toplevels output */
//     struct mwc_workspace *workspace = toplevel->workspace;
//     struct mwc_output *output = toplevel->workspace->output;
//     struct mwc_output *relative_output = output_get_relative(toplevel->workspace->output, direction);
//
//     if(toplevel->fullscreen) {
//         struct mwc_output *relative_output = output_get_relative(output, direction);
//         if(relative_output != NULL) {
//             focus_output(relative_output, opposite_side);
//         }
//         return;
//     }
//
//     if(toplevel->floating) {
//         struct mwc_toplevel *closest = toplevel_find_closest_floating_on_workspace(toplevel, direction);
//         if(closest != NULL) {
//             focus_toplevel(closest);
//             cursor_jump_focused_toplevel();
//             return;
//         }
//         struct mwc_output *relative_output = output_get_relative(output, direction);
//         if(relative_output != NULL) {
//             focus_output(relative_output, opposite_side);
//         }
//         return;
//     }
//
//     struct wl_list *next;
//     if(toplevel_is_master(toplevel)) {
//         switch(direction) {
//             case MWC_RIGHT: {
//                 next = toplevel->link.next;
//                 if(next == &workspace->masters) {
//                     next = workspace->slaves.prev;
//                     if(next == &workspace->slaves) {
//                         if(relative_output != NULL) {
//                             focus_output(relative_output, opposite_side);
//                         }
//                         return;
//                     }
//                 }
//                 struct mwc_toplevel *t = wl_container_of(next, t, link);
//                 focus_toplevel(t);
//                 cursor_jump_focused_toplevel();
//                 return;
//             }
//             case MWC_LEFT: {
//                 next = toplevel->link.prev;
//                 if(next == &workspace->masters) {
//                     if(relative_output != NULL) {
//                         focus_output(relative_output, opposite_side);
//                     }
//                     return;
//                 }
//                 struct mwc_toplevel *t = wl_container_of(next, t, link);
//                 focus_toplevel(t);
//                 cursor_jump_focused_toplevel();
//                 return;
//             }
//             default: {
//                 if(relative_output != NULL) {
//                     focus_output(relative_output, opposite_side);
//                 }
//                 return;
//             }
//         }
//     }
//
//     /* only case left is that the toplevel is a slave */
//     switch(direction) {
//         case MWC_LEFT: {
//             struct mwc_toplevel *last_master = wl_container_of(workspace->masters.prev, last_master, link);
//             focus_toplevel(last_master);
//             cursor_jump_focused_toplevel();
//             return;
//         }
//         case MWC_RIGHT: {
//             if(relative_output != NULL) {
//                 focus_output(relative_output, opposite_side);
//             }
//             return;
//         }
//         case MWC_UP: {
//             struct wl_list *above = toplevel->link.prev;
//             if(above == &workspace->slaves) {
//                 if(relative_output != NULL) {
//                     focus_output(relative_output, opposite_side);
//                 }
//                 return;
//             }
//             struct mwc_toplevel *t = wl_container_of(above, t, link);
//             focus_toplevel(t);
//             cursor_jump_focused_toplevel();
//             return;
//         }
//         case MWC_DOWN: {
//             struct wl_list *bellow = toplevel->link.next;
//             if(bellow == &workspace->slaves) {
//                 if(relative_output != NULL) {
//                     focus_output(relative_output, opposite_side);
//                 }
//                 return;
//             }
//             struct mwc_toplevel *t = wl_container_of(bellow, t, link);
//             focus_toplevel(t);
//             cursor_jump_focused_toplevel();
//             return;
//         }
//     }
// }
//
// void
// keybind_swap_focused_toplevel(void *data) {
//     uint64_t direction = (uint64_t)data;
//
//     struct mwc_toplevel *toplevel = server.focused_toplevel;
//     if(toplevel == NULL || toplevel == server.grabbed_toplevel)
//         return;
//
//     struct mwc_workspace *workspace = toplevel->workspace;
//     struct mwc_output *relative_output = output_get_relative(workspace->output, direction);
//
//     if(toplevel->floating || toplevel->fullscreen) {
//         if(relative_output != NULL && relative_output->active_workspace->fullscreen_toplevel == NULL) {
//             toplevel_move_to_workspace(toplevel, relative_output->active_workspace);
//         }
//         return;
//     }
//
//     struct wl_list *next;
//     if(toplevel_is_master(toplevel)) {
//         switch(direction) {
//             case MWC_RIGHT: {
//                 next = toplevel->link.next;
//                 if(next == &workspace->masters) {
//                     next = workspace->slaves.prev;
//                     if(next == &workspace->slaves) {
//                         if(relative_output != NULL && relative_output->active_workspace->fullscreen_toplevel == NULL)
//                         {
//                             toplevel_move_to_workspace(toplevel, relative_output->active_workspace);
//                         }
//                         return;
//                     }
//                 }
//                 struct mwc_toplevel *t = wl_container_of(next, t, link);
//                 layout_swap_tiled_toplevels(toplevel, t);
//                 return;
//             }
//             case MWC_LEFT: {
//                 next = toplevel->link.prev;
//                 if(next == &workspace->masters) {
//                     if(relative_output != NULL && relative_output->active_workspace->fullscreen_toplevel == NULL) {
//                         toplevel_move_to_workspace(toplevel, relative_output->active_workspace);
//                     }
//                     return;
//                 }
//                 struct mwc_toplevel *t = wl_container_of(next, t, link);
//                 layout_swap_tiled_toplevels(t, toplevel);
//                 return;
//             }
//             default: {
//                 struct mwc_output *relative_output = output_get_relative(workspace->output, direction);
//                 if(relative_output != NULL && relative_output->active_workspace->fullscreen_toplevel == NULL) {
//                     toplevel_move_to_workspace(toplevel, relative_output->active_workspace);
//                 }
//                 return;
//             }
//         }
//     }
//
//     switch(direction) {
//         case MWC_LEFT: {
//             struct mwc_toplevel *last_master = wl_container_of(workspace->masters.prev, last_master, link);
//             layout_swap_tiled_toplevels(toplevel, last_master);
//             return;
//         }
//         case MWC_RIGHT: {
//             struct mwc_output *relative_output = output_get_relative(workspace->output, direction);
//             if(relative_output != NULL && relative_output->active_workspace->fullscreen_toplevel == NULL) {
//                 toplevel_move_to_workspace(toplevel, relative_output->active_workspace);
//             }
//             return;
//         }
//         case MWC_UP: {
//             next = toplevel->link.prev;
//             if(next == &workspace->slaves) {
//                 if(relative_output != NULL && relative_output->active_workspace->fullscreen_toplevel == NULL) {
//                     toplevel_move_to_workspace(toplevel, relative_output->active_workspace);
//                 }
//                 return;
//             }
//             struct mwc_toplevel *t = wl_container_of(next, t, link);
//             layout_swap_tiled_toplevels(t, toplevel);
//             return;
//         }
//         case MWC_DOWN: {
//             next = toplevel->link.next;
//             if(next == &workspace->slaves) {
//                 if(relative_output != NULL && relative_output->active_workspace->fullscreen_toplevel == NULL) {
//                     toplevel_move_to_workspace(toplevel, relative_output->active_workspace);
//                 }
//                 return;
//             }
//             struct mwc_toplevel *t = wl_container_of(next, t, link);
//             layout_swap_tiled_toplevels(toplevel, t);
//             return;
//         }
//     }
// }
