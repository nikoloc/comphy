#include "rules.h"

#include "util/memory.h"

void
toplevel_rule_destroy(struct toplevel_rule *rule) {
    wl_list_remove(&rule->link);

    FREE(rule->match.app_id);
    FREE(rule->match.title);
    FREE(rule);
}

void
pointer_rule_destroy(struct pointer_rule *rule) {
    wl_list_remove(&rule->link);

    FREE(rule->match.name);
    FREE(rule);
}
