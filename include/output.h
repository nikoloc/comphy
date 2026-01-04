#ifndef OUTPUT_H
#define OUTPUT_H

#include <wlr/types/wlr_output.h>

#include "backend.h"
#include "ints.h"

struct output_config {
    u32 width, height, refresh;
    // ...
};

struct output {
    struct wlr_output *wlr_output;
};

struct output_manager {
    struct backend *backend;
    struct wlr_output_layout *wlr_output_layout;

    struct wl_list outputs;

    struct wl_listener new_output;
};

struct output_manager *
output_manager_create(struct backend *backend);

void
output_manager_destroy(struct output_manager *output_manager);

struct output *
output_manager_add_output(struct output_manager *output_manager, struct wlr_output *wlr_output);

void
output_manager_remove_output(struct output_manager *output_manager, struct output *output);

bool
output_configure(struct output *output, struct output_config *output_config);

#endif
