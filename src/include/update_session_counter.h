#ifndef __UPATE_SESSION_COUNTER_H__
#define __UPATE_SESSION_COUNTER_H__

#include "defs.h"

static inline void update_session_counter(union protocol_base_cb *__u){
	        switch(__u->none.proto){
            case proto_udp_t:{
                unsigned s = 0;
                struct udp_node *p = &(__u->udp);
                struct node *n = p->node, *n0 = n + p->node_count;
                for(; n < n0; ++n)
                    s += n->value_size;
                p->node_count = s;
            }
                break;
            case proto_tcp_t:{
                unsigned s = 0;
                struct tcp_node *p = &(__u->tcp);
                struct node *n = p->node, *n0 = n + p->node_count;
                for(; n < n0; ++n)
                    s += n->value_size;
                p->node_count = s;
            }
                break;
            default:
                break; /* Stupid break */
        }
}
#endif