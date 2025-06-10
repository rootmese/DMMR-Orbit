#include <dmmr_socket.h>
#include <socket_tcp.h>
#include <socket_udp.h>

static union protocol_base_cb *cap = 0;
static unsigned cap_size = 0;
static unsigned cap_count = 0;

static struct dmmr_socket *this = 0;

// TODO Avoid memleak recicle closed connection
/*
    Something like this:
       union protocol_base_cb *p = cap; *p1 = p + cap_size;
       do{
            if(!p)
                return p;
       }while(++p < p1);
*/
union protocol_base_cb *get_union_protocol_base_cb(void) {
    if(tcp_pool_count >= tcp_pool_size) {
        cap_size *= 2;
        cap = (union protocol_base_cb*)realloc(cap, cap_size * sizeof(union protocol_base_cb));
        if(!cap)
            return 0;
    }
    return cap + cap_count++;
}

static void dispatcher_cb_udp(struct udp_node *node){
    if(node){
        union protocol_base_cb *u = get_union_protocol_base_cb();
        if(u){
            if(this->dispatcher_cb){
                u->udp = node;
                this->dispatcher_cb(u);
            }
                
        }
    }
}

static void dispatcher_cb_tcp(struct tcp_node *node){
    if(node){
        union protocol_base_cb *u = get_union_protocol_base_cb();
        if(u){
            if(this->dispatcher_cb){
                u->tcp = node;
                this->dispatcher_cb(u);
            }
                
        }
    }
}

static int dmmr_start_acception(proto_t proto, ezp_addr_type family, uint16_t port, const char* ip) {
    int ret;
    union protocol_base_cb *cb;

    switch (proto) {
        case proto_udp_t:
            cb = get_union_protocol_base_cb();
            cb->udp = get_udp_node();
            if(cb.udp){
                cb->udp.proto = proto_udp_t;
                cb->udp.node.family = family;
                cb->udp.node.port = port;
            }
            break;
        case proto_tcp_t:{
            unsigned char buf[sizeof(struct sockaddr_in)];
            cb = get_union_protocol_base_cb();
            cb.tcp = get_tcp_node();
            if(cb->tcp){
                ezp_addr_type tyte = dns2ipaddr(ip, buf);
                switch(type){
                    case EZP_IPV4:
                        __vcpy(&(cb->tcp->proto.ipv4), buf, sizeof(struct sockaddr));
                        break;
                    case AF_INET6:
                        __vcpy(&(cb->tcp->proto.ipv6), buf, sizeof(struct sockaddr_in6));
                        brea;
                    default:
                        break;
                }
                cb->tcp->proto = proto_tcp_t;
                cb->tcp->port = port;
                cb->tcp->node.family = family;
                cb->tcp->on_accept_cb = this->acception_cb;
                cb->tcp->dispatcher_cb = dispatcher_cb_tcp;
                ret = start_tcp_service(cb->tcp);
                //TODO Error treatment
            }
        }
            break;
        }
        case proto_none_udp:
        default:
            break;
    }

    if (global_socket_instance && global_socket_instance->acception_cb) {
        global_socket_instance->acception_cb(cb);
    }

    return 0;
}

static void reload(struct cfg_server_server* __cfg_server){
}

struct dmmr_socket *new_dmmr_socket(struct cfg_server_server *cfg, void (*acception_cb)(struct node*), void (*acception_cb)(union protocol_base_cb*)){
    if(this)
        return 0;
	struct dmmr_socket *p = (struct dmmr_socket*)calloc(1, sizeof(struct dmmr_socket));
	if(p){
        cap = (union protocol_base_cb*)calloc(1, sizeof(union protocol_base_cb));
        if(!cap)
            goto error;
        cap_size = 0x400;
		p->acception_cb  = acception_cb;
		p->dispatcher_cb = dispatcher_cb;
        this = p;
        return p;
	}
    error:
       if(p)
        free(p);
        return EOF;
}
