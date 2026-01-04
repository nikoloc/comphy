#include "output.h"

#include <wlr/types/wlr_output_layout.h>

#include "alloc.h"
#include "macros.h"

static void
handle_new_output(struct wl_listener *listener, void *data) {
    todo();
}

struct output_manager *
output_manager_create(struct backend *backend) {
    struct output_manager *output_manager = alloc(sizeof(*output_manager));

    output_manager->backend = backend;
    output_manager->wlr_output_layout = wlr_output_layout_create(backend->display);

    wl_list_init(&output_manager->outputs);

    output_manager->new_output.notify = handle_new_output;
    wl_signal_add(&backend->wlr_backend->events.new_output, &output_manager->new_output);

    return output_manager;
}

void
output_manager_destroy(struct output_manager *output_manager);

struct output *
output_manager_add_output(struct output_manager *output_manager, struct wlr_output *wlr_output);

void
output_manager_remove_output(struct output_manager *output_manager, struct output *output);

bool
output_configure(struct output *output, struct output_config *output_config);
