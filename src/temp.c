// static void
// handle_action(int fd, char *action, char **args, size_t arg_count) {
//     if(strcmp(action, "exit") == 0) {
//         keybind_stop_server(NULL);
//     } else if(strcmp(action, "run") == 0) {
//         if(arg_count < 1)
//             goto done;
//
//         keybind_run(args[0]);
//     } else if(strcmp(action, "close") == 0) {
//         keybind_close(NULL);
//     } else if(strcmp(action, "toggle_floating") == 0) {
//         keybind_toggle_floating(NULL);
//     } else if(strcmp(action, "move_focus") == 0) {
//         if(arg_count < 1)
//             goto done;
//
//         enum direction direction;
//         if(strcmp(args[0], "up") == 0) {
//             direction = DIRECTION_UP;
//         } else if(strcmp(args[0], "left") == 0) {
//             direction = DIRECTION_LEFT;
//         } else if(strcmp(args[0], "down") == 0) {
//             direction = DIRECTION_DOWN;
//         } else if(strcmp(args[0], "right") == 0) {
//             direction = DIRECTION_RIGHT;
//         } else {
//             goto done;
//         }
//
//         keybind_move_focus((void *)direction);
//     } else if(strcmp(action, "move") == 0) {
//         if(arg_count < 1)
//             goto done;
//
//         enum direction direction;
//         if(strcmp(args[0], "up") == 0) {
//             direction = DIRECTION_UP;
//         } else if(strcmp(args[0], "left") == 0) {
//             direction = DIRECTION_LEFT;
//         } else if(strcmp(args[0], "down") == 0) {
//             direction = DIRECTION_DOWN;
//         } else if(strcmp(args[0], "right") == 0) {
//             direction = DIRECTION_RIGHT;
//         } else {
//             goto done;
//         }
//
//         keybind_move((void *)direction);
//     } else if(strcmp(action, "workspace") == 0) {
//         if(arg_count < 1)
//             goto done;
//
//         keybind_change_workspace((void *)(intptr_t)atoi(args[0]));
//     } else if(strcmp(action, "move_to_workspace") == 0) {
//         if(arg_count < 1)
//             goto done;
//
//         keybind_move_to_workspace((void *)(intptr_t)atoi(args[0]));
//     } else if(strcmp(action, "next_workspace") == 0) {
//         keybind_next_workspace(NULL);
//     } else if(strcmp(action, "prev_workspace") == 0) {
//         keybind_prev_workspace(NULL);
//     } else if(strcmp(action, "toggle_fullscreen") == 0) {
//         keybind_toggle_fullscreen(NULL);
//     } else if(strcmp(action, "toggle_fake_fullscreen") == 0) {
//         keybind_toggle_fake_fullscreen(NULL);
//     } else if(strcmp(action, "master_ratio") == 0) {
//         if(arg_count < 1)
//             goto done;
//
//         if(args[0][1] == '+') {
//             keybind_adjust_master_ratio((void *)(intptr_t)(atof(&args[0][1]) * 10000));
//         } else if(args[0][1] == '-') {
//             keybind_adjust_master_ratio((void *)(intptr_t)(-atof(&args[0][1]) * 10000));
//         } else {
//             keybind_set_master_ratio((void *)(intptr_t)(atof(args[0]) * 10000));
//         }
//     }
//
// done:
//     close(fd);
// }
//
// void
// action_perform(struct state *state, enum action_type type, void *_action) {
//     switch(type) {
//         case ACTION_TYPE_CURSOR: {
//             struct action_cursor *action = _action;
//
//             if(action->theme && action->size > 0) {
//                 free(state->config.cursor.theme);
//                 state->config.cursor.theme = strdup(action->theme);
//                 state->config.cursor.size = action->size;
//
//                 cursor_set_theme(&state->cursor, action->theme, action->size);
//             }
//
//             // TODO: see about this
//             // state->config.warp_on_output_change = warp_on_output_change;
//
//             // NOTE: to disable this you are supposed to pass -1, or something negative
//             if(action->hide_after) {
//                 state->config.cursor.hide_after = action->hide_after;
//             }
//
//             break;
//         }
//         case ACTION_TYPE_GAPS: {
//             struct action_gaps *action = _action;
//             break;
//         }
//         case ACTION_TYPE_BORDER: {
//             break;
//         }
//         case ACTION_TYPE_TOPLEVEL: {
//             break;
//         }
//         case ACTION_TYPE_ADJUST_MASTER_RATIO: {
//             break;
//         }
//         case ACTION_TYPE_SET_MASTER_RATIO: {
//             break;
//         }
//         case ACTION_TYPE_CREATE_KEYBINDS: {
//             break;
//         }
//     }
// }
//
