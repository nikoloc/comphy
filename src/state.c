#include "state.h"

struct state *
state_get(void) {
    static struct state state = {0};

    return &state;
}
