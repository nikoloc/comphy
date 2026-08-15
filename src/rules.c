#include "rules.h"

#include "util/memory.h"

void
toplevel_rule_destroy(struct toplevel_rule *rule) {
    FREE(rule->match.app_id);
    FREE(rule->match.title);
    FREE(rule);
}

void
pointer_rule_destroy(struct pointer_rule *rule) {
    FREE(rule->match.name);
    FREE(rule);
}

void
output_rule_destroy(struct output_rule *rule) {
    FREE(rule->match.name);
    FREE(rule);
}
