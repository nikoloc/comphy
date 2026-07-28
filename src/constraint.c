#include "constraint.h"

// /* a lot of the code was stolen of labwc's implemenetation, big props to them */
// void
// server_handle_new_constraint(struct wl_listener *listener, void *data) {
// 	struct wlr_pointer_constraint_v1 *wlr_constraint = data;
//
//   /* if there is already a constraint on this surface we ignore it */
//   struct wlr_pointer_constraint_v1 *con;
//   wl_list_for_each(con, &server.pointer_contrains_manager->constraints, link) {
//     if(con != wlr_constraint && con->surface == wlr_constraint->surface) return;
//   }
//
//   struct mwc_pointer_constraint *constraint = calloc(1, sizeof(*constraint));
//   constraint->wlr_pointer_constraint = wlr_constraint;
//   constraint->wlr_pointer_constraint->data = constraint;
//
//   constraint->destroy.notify = constraint_handle_destroy;
//   wl_signal_add(&wlr_constraint->events.destroy, &constraint->destroy);
// }
//
// void
// constraint_remove_current(void) {
//   if(server.current_constraint == NULL) return;
//
//   constraint_move_to_hint(server.current_constraint);
//
//   server.current_constraint = NULL;
//   wlr_pointer_constraint_v1_send_deactivated(server.current_constraint->wlr_pointer_constraint);
// }
//
// void
// constraint_set_as_current(struct mwc_pointer_constraint *constraint) {
//   if(server.current_constraint == constraint) return;
//
//   if(server.current_constraint != NULL) {
//     wlr_pointer_constraint_v1_send_deactivated(server.current_constraint->wlr_pointer_constraint);
//   }
//
//   server.current_constraint = constraint;
//   constraint_move_to_hint(constraint);
//   wlr_pointer_constraint_v1_send_activated(constraint->wlr_pointer_constraint);
// }
//
// void
// constraint_move_to_hint(struct mwc_pointer_constraint *constraint) {
//   struct wlr_pointer_constraint_v1 *wlr_constraint = constraint->wlr_pointer_constraint;
//
//   if(wlr_constraint->current.committed & WLR_POINTER_CONSTRAINT_V1_STATE_CURSOR_HINT) {
//     double sx = wlr_constraint->current.cursor_hint.x;
//     double sy = wlr_constraint->current.cursor_hint.y;
//     wlr_cursor_warp(server.cursor, NULL,
//                     X(server.focused_toplevel) + sx,
//                     Y(server.focused_toplevel) + sy);
//
//     /* make sure we are not sending unnecessary surface movements (took from labwc)*/
//     wlr_seat_pointer_warp(server.seat, sx, sy);
//   }
// }
//
// void
// constraint_handle_destroy(struct wl_listener *listener, void *data) {
// 	struct mwc_pointer_constraint *constraint = wl_container_of(listener, constraint, destroy);
//
// 	wl_list_remove(&constraint->destroy.link);
// 	if(server.current_constraint == constraint) {
//     constraint_move_to_hint(constraint);
//     server.current_constraint = NULL;
// 	}
//
// 	free(constraint);
// }
//
// void
// constrain_apply_to_move(double *dx, double *dy) {
//   if(server.current_constraint == NULL) return;
//
//   if(server.current_constraint->wlr_pointer_constraint->type == WLR_POINTER_CONSTRAINT_V1_LOCKED) {
//     *dx = 0;
//     *dy = 0;
//     return;
//   }
//
//   if(server.seat->pointer_state.focused_surface == NULL) return;
//
// 	double current_x = server.seat->pointer_state.sx;
// 	double current_y = server.seat->pointer_state.sy;
//
//   double constrained_x, constrained_y;
//   if(wlr_region_confine(&server.current_constraint->wlr_pointer_constraint->region,
//                      current_x, current_y,
//                      current_x + *dx, current_y + *dy,
//                      &constrained_x, &constrained_y)) {
//     *dx = constrained_x - current_x;
//     *dy = constrained_y - current_y;
//   }
// }
//
// void
// server_handle_relative_pointer_manager_destroy(struct wl_listener *listener, void *data) {
//   wl_list_remove(&server.relative_pointer_manager_destroy.link);
// }
