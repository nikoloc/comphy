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

#include "keybind.h"
#include "util/memory.h"

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

    config->keyboard.rate = 20;
    config->keyboard.delay = 200;

    wl_list_init(&config->pointer_rules);
    wl_list_init(&config->toplevel_rules);

    config->master_ratio = 0.5f;
}

void
config_deinit(struct config *config) {
    {
        struct keybind *iter, *temp;
        wl_list_for_each_safe(iter, temp, &config->keybinds, link) {
            keybind_destroy(iter);
        }
    }

    FREE(config->keyboard.xkb_layouts);
    FREE(config->keyboard.xkb_variants);
    FREE(config->keyboard.xkb_options);

    // TODO: pointer_rules and toplevel_rules
}
