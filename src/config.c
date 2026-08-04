#include "config.h"

#include <assert.h>
#include <libinput.h>
#include <limits.h>
#include <regex.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/util/log.h>

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

void
config_init(struct config *config) {
    wl_list_init(&config->keybinds);
    wl_list_init(&config->pointer_keybinds);

    config->keyboard.rate = 20;
    config->keyboard.delay = 200;

    wl_list_init(&config->pointer_rules);
    wl_list_init(&config->toplevel_rules);

    config->opacity.active = 1;
    config->opacity.inactive = 1;

    config->master_ratio = 0.5f;
}

void
config_deinit(struct config *config) {
    // TODO
}
