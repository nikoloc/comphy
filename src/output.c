#include "output.h"

#include <limits.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

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
output_manager_destroy(struct output_manager *output_manager) {
    wlr_output_layout_destroy(output_manager->wlr_output_layout);
    free(output_manager);
}

struct output *
output_manager_add_output(struct output_manager *output_manager, struct wlr_output *wlr_output) {
    struct output *output = alloc(sizeof(*output));
    output->wlr_output = wlr_output;
    wl_list_insert(&output_manager->outputs, &output->link);

    return output;
}

void
output_manager_remove_output(struct output_manager *output_manager, struct output *output) {
    wlr_output_layout_remove(output_manager->wlr_output_layout, output->wlr_output);
    free(output);
}

// we try to find the closest supported mode for this output, that means:
//     - same resolution
//     - closest refresh rate (since a lot of times its like 59.994 or something)
static struct wlr_output_mode *
find_mode(struct output *output, int width, int height, int refresh) {
    struct wlr_output_mode *mode = NULL;

    struct wlr_output_mode *iter;
    wl_list_for_each(iter, &output->wlr_output->modes, link) {
        if(iter->width == width && iter->height == height &&
                (mode == NULL || abs(iter->refresh - refresh) < abs(mode->refresh - refresh))) {
            mode = iter;
        }
    }

    if(mode) {
        return mode;
    }

    return wlr_output_preferred_mode(output->wlr_output);
}

static struct wlr_output_mode *
find_highest_refresh(struct output *output, int width, int height) {
    struct wlr_output_mode *mode = NULL;

    struct wlr_output_mode *iter;
    wl_list_for_each(iter, &output->wlr_output->modes, link) {
        if(iter->width == width && iter->height == height && (mode == NULL || iter->refresh > mode->refresh)) {
            mode = iter;
        }
    }

    if(mode) {
        return mode;
    }

    return wlr_output_preferred_mode(output->wlr_output);
}

static void
modeset(struct output *output, int width, int height, int refresh, double scale) {
    bool wants_preferred = width == 0 || height == 0;
    bool wants_highest_refresh = refresh == 0;

    struct wlr_output_mode *mode = wants_preferred       ? wlr_output_preferred_mode(output->wlr_output)
                                 : wants_highest_refresh ? find_highest_refresh(output, width, height)
                                                         : find_mode(output, width, height, refresh);

    wlr_log(WLR_INFO, "modesetting output `%s` to %dx%d@%dmHz", output->wlr_output->name, mode->width, mode->height,
            mode->refresh);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    wlr_output_state_set_scale(&state, scale);
    wlr_output_state_set_mode(&state, mode);

    // try to commit the state. it should not fail!
    if(!wlr_output_commit_state(output->wlr_output, &state)) {
        wlr_log(WLR_ERROR, "could not modeset to the output `%s`", output->wlr_output->name);
    }

    wlr_output_state_finish(&state);
}

bool
output_configure(struct output *output, struct output_config *output_config) {
    // TODO: maybe move this to configure?
    struct wlr_output_layout_output *layout_output =
            wlr_output_layout_add_auto(output->output_manager->wlr_output_layout, output->wlr_output);

    modeset(output, output_config->width, output_config->height, output_config->refresh, output_config->scale);

    struct wlr_output_layout_output *wlr_layout_output =
            wlr_output_layout_add_auto(output->output_manager->wlr_output_layout, output->wlr_output);

    // TODO: fix
    // wlr_scene_output_layout_add_output(server.scene_layout, layout, output->scene_output);

    wlr_output_layout_get_box(output->output_manager->wlr_output_layout, output->wlr_output, &output->box);

    // if(initial) {
    //     assign_workspaces(output, &config);
    // }

    return true;
}
