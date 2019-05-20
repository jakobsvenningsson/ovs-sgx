#include "ofpacts.h"


/* Returns true if 'action' outputs to 'port', false otherwise. */
static bool
ofpact_outputs_to_port(const struct ofpact *ofpact, uint16_t port)
{
    switch (ofpact->type) {
    case OFPACT_OUTPUT:
        return ofpact_get_OUTPUT(ofpact)->port == port;
    case OFPACT_ENQUEUE:
        return ofpact_get_ENQUEUE(ofpact)->port == port;
    case OFPACT_CONTROLLER:
        return port == OFPP_CONTROLLER;
    default:
        return false;
    }
}

bool
ofpacts_output_to_port(const struct ofpact *ofpacts, size_t ofpacts_len,
                       uint16_t port)
{
    const struct ofpact *a;

    OFPACT_FOR_EACH (a, ofpacts, ofpacts_len) {
        if (ofpact_outputs_to_port(a, port)) {
            return true;
        }
    }

    return false;
}


bool
ofproto_rule_has_out_port(const struct rule *rule, uint16_t port)
{
    return (port == OFPP_ANY
            || ofpacts_output_to_port(rule->ofpacts, rule->ofpacts_len, port));
}
