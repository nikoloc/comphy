#ifndef ACTION_H
#define ACTION_H

#include <libinput.h>
#include <wlr/types/wlr_output_layout.h>

#include "action_types.h"
#include "toplevel.h"
#include "util/shell_parser.h"

// these next two functions are ai generated in `action_parser.c` from the specification found in the
// `comphyctl_commands.txt`
bool
action_create(struct shell_parser *parser, enum action_type *out_type, void **dest);

void
action_destroy(enum action_type type, void *_action);

void
action_perform(struct state *state, enum action_type type, void *action);

#endif
