
#ifndef _OFPACTS_H_
#define _OFPACTS_H_

#include "openflow-common.h"
#include <stdint.h>
#include <stddef.h>
#include "list.h"
#include "ofproto-provider.h"



enum ofp_port {
    /* Ranges. */
    OFPP_MAX        = 0xff00,   /* Maximum number of physical switch ports. */
    OFPP_FIRST_RESV = 0xfff8,   /* First assigned reserved port number. */
    OFPP_LAST_RESV  = 0xffff,   /* Last assigned reserved port number. */

    /* Reserved output "ports". */
    OFPP_IN_PORT    = 0xfff8,  /* Send the packet out the input port.  This
                                  virtual port must be explicitly used
                                  in order to send back out of the input
                                  port. */
    OFPP_TABLE      = 0xfff9,  /* Perform actions in flow table.
                                  NB: This can only be the destination
                                  port for packet-out messages. */
    OFPP_NORMAL     = 0xfffa,  /* Process with normal L2/L3 switching. */
    OFPP_FLOOD      = 0xfffb,  /* All physical ports except input port and
                                  those disabled by STP. */
    OFPP_ALL        = 0xfffc,  /* All physical ports except input port. */
    OFPP_CONTROLLER = 0xfffd,  /* Send to controller. */
    OFPP_LOCAL      = 0xfffe,  /* Local openflow "port". */
    OFPP_NONE       = 0xffff   /* Not associated with a physical port. */
};


enum ofpbuf_source {
    OFPBUF_MALLOC,              /* Obtained via malloc(). */
    OFPBUF_STACK,               /* Un-movable stack space or static buffer. */
    OFPBUF_STUB                 /* Starts on stack, may expand into heap. */
};

struct ofpbuf {
    void *base;                 /* First byte of allocated space. */
    size_t allocated;           /* Number of bytes allocated. */
    enum ofpbuf_source source;  /* Source of memory allocated as 'base'. */

    void *data;                 /* First byte actually in use. */
    size_t size;                /* Number of bytes in use. */

    void *l2;                   /* Link-level header. */
    void *l2_5;                 /* MPLS label stack */
    void *l3;                   /* Network-level header. */
    void *l4;                   /* Transport-level header. */
    void *l7;                   /* Application data. */

    struct list list_node;      /* Private list element for use by owner. */
    void *private_p;            /* Private pointer for use by owner. */
};




#define OFPACT_FOR_EACH(POS, OFPACTS, OFPACTS_LEN)                      \
    for ((POS) = (OFPACTS); (POS) < ofpact_end(OFPACTS, OFPACTS_LEN);  \
         (POS) = ofpact_next(POS))

#define OFPACTS                                                     \
    /* Output. */                                                   \
    DEFINE_OFPACT(OUTPUT,          ofpact_output,        ofpact)    \
    DEFINE_OFPACT(CONTROLLER,      ofpact_controller,    ofpact)    \
    DEFINE_OFPACT(ENQUEUE,         ofpact_enqueue,       ofpact)


/* enum ofpact_type, with a member OFPACT_<ENUM> for each action. */
enum OVS_PACKED_ENUM ofpact_type {
#define DEFINE_OFPACT(ENUM, STRUCT, MEMBER) OFPACT_##ENUM,
    OFPACTS
#undef DEFINE_OFPACT
};

void *ofpact_put(struct ofpbuf *, enum ofpact_type, size_t len);


#define OFPACT_ALIGNTO 8
#define OFPACT_ALIGN(SIZE) ROUND_UP(SIZE, OFPACT_ALIGNTO)


enum OVS_PACKED_ENUM ofputil_action_code {
    OFPUTIL_ACTION_INVALID,
#define OFPAT10_ACTION(ENUM, STRUCT, NAME)             OFPUTIL_##ENUM,
#define OFPAT11_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME) OFPUTIL_##ENUM,
#define NXAST_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME)   OFPUTIL_##ENUM,
//#include "ofp-util.def"
};

#define OFPP_ANY OFPP_NONE

struct ofpact {
    enum ofpact_type type;      /* OFPACT_*. */
    enum ofputil_action_code compat; /* Original type when added, if any. */
    uint16_t len;               /* Length of the action, in bytes, including
                                 * struct ofpact, excluding padding. */
};

struct ofpact_output {
   struct ofpact ofpact;
   uint16_t port;              /* Output port. */
   uint16_t max_len;           /* Max send len, for port OFPP_CONTROLLER. */
};

/* OFPACT_CONTROLLER.
*
* Used for NXAST_CONTROLLER. */
struct ofpact_controller {
   struct ofpact ofpact;
   uint16_t max_len;           /* Maximum length to send to controller. */
   uint16_t controller_id;     /* Controller ID to send packet-in. */
   enum ofp_packet_in_reason reason; /* Reason to put in packet-in. */
};

/* OFPACT_ENQUEUE.
*
* Used for OFPAT10_ENQUEUE. */
struct ofpact_enqueue {
   struct ofpact ofpact;
   uint16_t port;
   uint32_t queue;
};

static inline struct ofpact *
ofpact_next(const struct ofpact *ofpact)
{
    return (void *) ((uint8_t *) ofpact + OFPACT_ALIGN(ofpact->len));
}

static inline struct ofpact *
ofpact_end(const struct ofpact *ofpacts, size_t ofpacts_len)
{
    return (void *) ((uint8_t *) ofpacts + ofpacts_len);
}


#define DEFINE_OFPACT(ENUM, STRUCT, MEMBER)                             \
    BUILD_ASSERT_DECL(offsetof(struct STRUCT, ofpact) == 0);            \
                                                                        \
    enum { OFPACT_##ENUM##_RAW_SIZE                                     \
           = (offsetof(struct STRUCT, MEMBER)                           \
              ? offsetof(struct STRUCT, MEMBER)                         \
              : sizeof(struct STRUCT)) };                               \
                                                                        \
    enum { OFPACT_##ENUM##_SIZE                                         \
           = ROUND_UP(OFPACT_##ENUM##_RAW_SIZE, OFPACT_ALIGNTO) };      \
                                                                        \
    static inline struct STRUCT *                                       \
    ofpact_get_##ENUM(const struct ofpact *ofpact)                      \
    {                                                                   \
        ovs_assert(ofpact->type == OFPACT_##ENUM);                      \
        return (struct STRUCT *) ofpact;                                \
    }                                                                   \
                                                                        \
    static inline struct STRUCT *                                       \
    ofpact_put_##ENUM(struct ofpbuf *ofpacts)                           \
    {                                                                   \
        return ofpact_put(ofpacts, OFPACT_##ENUM,                       \
                          OFPACT_##ENUM##_RAW_SIZE);                    \
    }                                                                   \
                                                                        \
    static inline void                                                  \
    ofpact_init_##ENUM(struct STRUCT *ofpact)                           \
    {                                                                   \
        ofpact_init(&ofpact->ofpact, OFPACT_##ENUM,                     \
                    OFPACT_##ENUM##_RAW_SIZE);                          \
    }
OFPACTS
#undef DEFINE_OFPACT


bool
ofproto_rule_has_out_port(const struct rule *rule, uint16_t port);

#endif
