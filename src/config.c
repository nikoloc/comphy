#include "config.h"

#include <assert.h>
#include <libinput.h>
#include <limits.h>
#include <regex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

#include "keybinds.h"
#include "keyboard.h"
#include "layer.h"
#include "layout.h"
#include "output.h"
#include "pointer.h"
#include "toplevel.h"
#include "util/memory.h"
#include "workspace.h"

void
config_add_pointer_rule(struct state *state, struct action_pointer *pointer) {
    struct pointer_rule *rule = ALLOCATE(struct pointer_rule);
    rule->pointer.name = strdup(pointer->name);
    rule->pointer.sensitivity = pointer->sensitivity;
    rule->pointer.acceleration = pointer->acceleration;
    rule->pointer.left_handed = pointer->left_handed;

    wl_list_insert(&state->config.pointer_rules, &rule->link);
}

void
config_add_toplevel_rule(struct state *state, struct action_toplevel *toplevel) {
    struct toplevel_rule *rule = ALLOCATE(struct toplevel_rule);
    rule->toplevel.app_id = strdup(toplevel->app_id);
    rule->toplevel.title = strdup(toplevel->title);
    rule->toplevel.floats = toplevel->floats;
    rule->toplevel.client_side_decorations = toplevel->client_side_decorations;
    rule->toplevel.opacity_active = toplevel->opacity_active;
    rule->toplevel.opacity_inactive = toplevel->opacity_inactive;
    rule->toplevel.default_state = toplevel->default_state;
    rule->toplevel.default_width = toplevel->default_width;
    rule->toplevel.default_height = toplevel->default_height;

    wl_list_insert(&state->config.pointer_rules, &rule->link);
}

void
config_init(struct config *config) {
    wl_list_init(&config->keybinds);
    wl_list_init(&config->pointer_keybinds);

    config->keyboard.rate = 150;
    config->keyboard.delay = 50;

    wl_list_init(&config->pointer_rules);
    wl_list_init(&config->toplevel_rules);

    config->opacity.active = 1;
    config->opacity.inactive = 1;

    config->master_ratio = 0.5f;
}

// bool
// config_add_keybind(struct mwc_config *c, char *modifiers, char *key, char *action, char **args, size_t arg_count) {
//     char *p = modifiers;
//     uint32_t modifiers_flag = 0;
//
//     while(*p != '\0') {
//         char mod[64] = {0};
//         char *q = mod;
//         while(*p != '+' && *p != '\0') {
//             *q = *p;
//             p++;
//             q++;
//         }
//
//         if(strcmp(mod, "alt") == 0) {
//             modifiers_flag |= WLR_MODIFIER_ALT;
//         } else if(strcmp(mod, "super") == 0) {
//             modifiers_flag |= WLR_MODIFIER_LOGO;
//         } else if(strcmp(mod, "ctrl") == 0) {
//             modifiers_flag |= WLR_MODIFIER_CTRL;
//         } else if(strcmp(mod, "shift") == 0) {
//             modifiers_flag |= WLR_MODIFIER_SHIFT;
//         }
//
//         if(*p == '+') {
//             p++;
//         }
//     }
//
//     uint32_t key_sym = 0;
//     bool pointer = false;
//     if(strncmp(key, "mouse_", 6) == 0) {
//         pointer = true;
//         key = key + 6;
//         if(strcmp(key, "left_click") == 0) {
//             key_sym = 272;
//         } else if(strcmp(key, "right_click") == 0) {
//             key_sym = 273;
//         } else if(strcmp(key, "middle_click") == 0) {
//             key_sym = 274;
//         } else {
//             key_sym = atoi(key);
//         }
//     } else if(strncmp(key, "pointer_", 8) == 0) {
//         pointer = true;
//         key = key + 8;
//         if(strcmp(key, "left_click") == 0) {
//             key_sym = 272;
//         } else if(strcmp(key, "right_click") == 0) {
//             key_sym = 273;
//         } else if(strcmp(key, "middle_click") == 0) {
//             key_sym = 274;
//         } else {
//             key_sym = atoi(key);
//         }
//     } else {
//         if(strcmp(key, "return") == 0 || strcmp(key, "enter") == 0) {
//             key_sym = XKB_KEY_Return;
//         } else if(strcmp(key, "backspace") == 0) {
//             key_sym = XKB_KEY_BackSpace;
//         } else if(strcmp(key, "delete") == 0) {
//             key_sym = XKB_KEY_Delete;
//         } else if(strcmp(key, "escape") == 0) {
//             key_sym = XKB_KEY_Escape;
//         } else if(strcmp(key, "tab") == 0) {
//             key_sym = XKB_KEY_Tab;
//         } else if(strcmp(key, "up") == 0) {
//             key_sym = XKB_KEY_Up;
//         } else if(strcmp(key, "down") == 0) {
//             key_sym = XKB_KEY_Down;
//         } else if(strcmp(key, "left") == 0) {
//             key_sym = XKB_KEY_Left;
//         } else if(strcmp(key, "right") == 0) {
//             key_sym = XKB_KEY_Right;
//         } else {
//             key_sym = xkb_keysym_from_name(key, 0);
//             if(key_sym == 0) {
//                 wlr_log(WLR_ERROR, "key %s doesn't seem right", key);
//                 return false;
//             }
//         }
//     }
//
//     struct keybind *k = calloc(1, sizeof(*k));
//     *k = (struct keybind){
//             .modifiers = modifiers_flag,
//             .key = key_sym,
//     };
//
//     /* this is true for most, needs to be set to false if otherwise */
//     k->initialized = true;
//
//     if(strcmp(action, "exit") == 0) {
//         k->action = keybind_stop_server;
//     } else if(strcmp(action, "run") == 0) {
//         if(arg_count < 1) {
//             wlr_log(WLR_ERROR, "invalid args to %s", action);
//             free(k);
//             return false;
//         }
//
//         k->action = keybind_run;
//         char *args_0_copy = strdup(args[0]);
//         k->args = args_0_copy;
//     } else if(strcmp(action, "kill_active") == 0) {
//         k->action = keybind_close_keyboard_focused_toplevel;
//     } else if(strcmp(action, "switch_floating_state") == 0 || strcmp(action, "toggle_floating") == 0) {
//         k->action = keybind_focused_toplevel_toggle_floating;
//     } else if(strcmp(action, "resize") == 0) {
//         k->action = keybind_resize_focused_toplevel;
//         k->stop = keybind_stop_resize_focused_toplevel;
//     } else if(strcmp(action, "move") == 0) {
//         k->action = keybind_move_focused_toplevel;
//         k->stop = keybind_stop_move_focused_toplevel;
//     } else if(strcmp(action, "move_focus") == 0) {
//         if(arg_count < 1) {
//             wlr_log(WLR_ERROR, "invalid args to %s", action);
//             free(k);
//             return false;
//         }
//
//         enum mwc_direction direction;
//         if(strcmp(args[0], "up") == 0) {
//             direction = MWC_UP;
//         } else if(strcmp(args[0], "left") == 0) {
//             direction = MWC_LEFT;
//         } else if(strcmp(args[0], "down") == 0) {
//             direction = MWC_DOWN;
//         } else if(strcmp(args[0], "right") == 0) {
//             direction = MWC_RIGHT;
//         } else {
//             wlr_log(WLR_ERROR, "invalid args to %s", action);
//             free(k);
//             return false;
//         }
//
//         k->action = keybind_move_focus;
//         k->args = (void *)direction;
//     } else if(strcmp(action, "swap") == 0) {
//         if(arg_count < 1) {
//             wlr_log(WLR_ERROR, "invalid args to %s", action);
//             free(k);
//             return false;
//         }
//
//         enum mwc_direction direction;
//         if(strcmp(args[0], "up") == 0) {
//             direction = MWC_UP;
//         } else if(strcmp(args[0], "left") == 0) {
//             direction = MWC_LEFT;
//         } else if(strcmp(args[0], "down") == 0) {
//             direction = MWC_DOWN;
//         } else if(strcmp(args[0], "right") == 0) {
//             direction = MWC_RIGHT;
//         } else {
//             wlr_log(WLR_ERROR, "invalid args to %s", action);
//             free(k);
//             return false;
//         }
//
//         k->action = keybind_swap_focused_toplevel;
//         k->args = (void *)direction;
//     } else if(strcmp(action, "workspace") == 0) {
//         if(arg_count < 1) {
//             wlr_log(WLR_ERROR, "invalid args to %s", action);
//             free(k);
//             return false;
//         }
//         k->action = keybind_change_workspace;
//         /* this is going to be overriden by the actual workspace that is needed for change_workspace() */
//         k->args = (void *)atoi(args[0]);
//         k->initialized = false;
//     } else if(strcmp(action, "move_to_workspace") == 0) {
//         if(arg_count < 1) {
//             wlr_log(WLR_ERROR, "invalid args to %s", action);
//             free(k);
//             return false;
//         }
//         k->action = keybind_move_focused_toplevel_to_workspace;
//         /* this is going to be overriden by the actual workspace that is needed for change_workspace() */
//         k->args = (void *)atoi(args[0]);
//         k->initialized = false;
//     } else if(strcmp(action, "next_workspace") == 0) {
//         k->action = keybind_next_workspace;
//     } else if(strcmp(action, "prev_workspace") == 0) {
//         k->action = keybind_prev_workspace;
//     } else if(strcmp(action, "toggle_fullscreen") == 0) {
//         k->action = keybind_focused_toplevel_toggle_fullscreen;
//     } else if(strcmp(action, "reload_config") == 0) {
//         k->action = keybind_reload_config;
//     } else {
//         wlr_log(WLR_ERROR, "invalid keybind action %s", action);
//         free(k);
//         return false;
//     }
//
//     if(pointer) {
//         wl_list_insert(&c->pointer_keybinds, &k->link);
//     } else {
//         wl_list_insert(&c->keybinds, &k->link);
//     }
//     return true;
// }

bool
get_config_path(char *dest, size_t size) {
    char *env_conf = getenv("MWC_CONFIG_PATH");
    if(env_conf != NULL) {
        strncpy(dest, env_conf, size);
        dest[size - 1] = 0;
        return true;
    }

    char *config_home = getenv("XDG_CONFIG_HOME");
    if(config_home != NULL) {
        snprintf(dest, size, "%s/mwc/mwc.conf", config_home);
        return true;
    }

    char *home = getenv("HOME");
    if(home != NULL) {
        snprintf(dest, size, "%s/.config/mwc/mwc.conf", home);
        return true;
    }

    return false;
}

struct mwc_config *
config_load() {
    struct mwc_config *c = calloc(1, sizeof(*c));

    FILE *config_file;
    char path[1024];
    if(get_config_path(path, sizeof(path))) {
        config_file = fopen(path, "r");
        if(config_file != NULL) {
            char *current = path;
            char *last_slash = NULL;
            while(*current != 0) {
                if(*current == '/') {
                    last_slash = current;
                }
                current++;
            }

            assert(last_slash != NULL);
            *last_slash = 0;
            c->dir = strdup(path);
        } else {
            wlr_log(WLR_INFO, "couldn't open the config file");
            get_default_config_path(path, sizeof(path));
            config_file = fopen(path, "r");
        }
    } else {
        wlr_log(WLR_INFO, "couldn't get config file path, backing to default config");
        get_default_config_path(path, sizeof(path));
        config_file = fopen(path, "r");
    }

    if(config_file == NULL) {
        wlr_log(WLR_ERROR, "couldn't open the default config file");
        free(c);
        return NULL;
    }

    wl_list_init(&c->keybinds);
    wl_list_init(&c->pointer_keybinds);
    wl_list_init(&c->outputs);
    wl_list_init(&c->workspaces);
    wl_list_init(&c->pointers);
    wl_list_init(&c->window_rules.floating);
    wl_list_init(&c->window_rules.size);
    wl_list_init(&c->window_rules.opacity);
    wl_list_init(&c->layer_rules.blur);

    return c;
}

void
config_destroy(struct mwc_config *c) {
    free(c->dir);

    struct output_config *o, *o_temp;
    wl_list_for_each_safe(o, o_temp, &c->outputs, link) {
        free(o->name);
        free(o);
    }

    struct keybind *k, *k_temp;
    wl_list_for_each_safe(k, k_temp, &c->keybinds, link) {
        if(k->action == keybind_run) {
            free(k->args);
        }
        free(k);
    }
    wl_list_for_each_safe(k, k_temp, &c->pointer_keybinds, link) {
        free(k);
    }

    struct window_rule_float *wrf, *wrf_temp;
    wl_list_for_each_safe(wrf, wrf_temp, &c->window_rules.floating, link) {
        if(wrf->condition.has_app_id_regex) {
            regfree(&wrf->condition.app_id_regex);
        }
        if(wrf->condition.has_title_regex) {
            regfree(&wrf->condition.title_regex);
        }
        free(wrf);
    }
    struct window_rule_size *wrs, *wrs_temp;
    wl_list_for_each_safe(wrs, wrs_temp, &c->window_rules.size, link) {
        if(wrs->condition.has_app_id_regex) {
            regfree(&wrs->condition.app_id_regex);
        }
        if(wrs->condition.has_title_regex) {
            regfree(&wrs->condition.title_regex);
        }
        free(wrs);
    }
    struct window_rule_opacity *wro, *wro_temp;
    wl_list_for_each_safe(wro, wro_temp, &c->window_rules.opacity, link) {
        if(wro->condition.has_app_id_regex) {
            regfree(&wro->condition.app_id_regex);
        }
        if(wro->condition.has_title_regex) {
            regfree(&wro->condition.title_regex);
        }
        free(wro);
    }

    struct layer_rule_blur *lrb, *lrb_temp;
    wl_list_for_each_safe(lrb, lrb_temp, &c->layer_rules.blur, link) {
        if(lrb->condition.has) {
            regfree(&lrb->condition.regex);
        }

        free(lrb);
    }

    free(c->keymap_layouts);
    free(c->keymap_variants);
    free(c->keymap_options);

    struct pointer_config *p, *p_temp;
    wl_list_for_each_safe(p, p_temp, &c->pointers, link) {
        free(p->name);
        free(p);
    }

    free(c->cursor_theme);

    free(c->baked_points);

    for(size_t i = 0; i < c->run_count; i++) {
        free(c->run[i]);
    }

    free(c);
}
