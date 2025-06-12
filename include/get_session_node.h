#ifndef __GET_SESSION_NODE_H__
#define __GET_SESSION_NODE_H__

static inline struct node* get_session_node(union protocol_base_cb *__u){
	        switch(__u->none.proto){
            case proto_udp_t:{
                struct udp_node *p = &(__u->udp);
                return &(p->node);
            }
                break;
            case proto_tcp_t{
                struct tcp_node *p = &(__u->udp);
                return &(p->node);
            }
                break;
            default:
                return 0;
                break; /* Stupid break */
        }
}

#endif