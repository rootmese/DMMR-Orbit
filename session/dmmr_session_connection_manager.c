#include <defs.h>
#include <dmmr_session_connection_manager.h>
#include <dmmr_circle_buffer.h>
#include <session_connection.h>

static struct circle_buffer* _circle_buffer = 0;
static struct dmmr_scheduler* _sched = 0;
static struct dmmr_socket *_sock = 0;
static struct dmmr_session_connection_manager _this = 0;

// Prototipa a função que vai sobrepor o enqueue
static void session_manager_enqueue(struct circle_buffer *cb, struct node_circle_buffer* n){
    struct session_connection_pool *pool;
    if(n)
        pool = insert_session(n, n->n.port);
        // pthread_mutex_lock(&(this->fifo_lock));
        // TAILQ_INSERT_TAIL(&(this->fifo), n, tailq);
        // pthread_mutex_unlock(&this->fifo_lock);
        // terá no circle_buffer um método chamado queue
}

static int listen(proto_t proto, uint16_t port, const char *host){
    if(host){
        int ret;
        char ip[sizeof(struct sockaddr_in6)];
        ezp_addr_type family =  dns2ipaddr(host, ip);
        switch(proto){
            case proto_udp_t:{
                union protocol_base_cb *u = get_union_protocol_base_cb();
                if(u){
                    struct udp_node *udp = &(u.udp);
                    if(udp){
                        udp->proto = proto;
                        switch(family){
                            case AF_INET:
                                ret = inet_pton(AF_INET, ip_str, &(udp->node.cfg.ipv4));
                                if(!ret){
                                    udp->node.cfg.family = family;
                                    udp->node.cfg.port = port;
                                    return start_udp_service(udp);
                                }
                                break;
                            case AF_INET6:
                                ret = inet_pton(AF_INET6, ip_str, &(udp->node.cfg.ipv6));
                                if(!ret){
                                    udp->node.cfg.family = family;
                                    udp->node.cfg.port = port;
                                    return start_udp_service(udp);
                                }
                                break;
                             default:
                                return EOF;
                        }
                        
                    }
                }
            }
            break;
        }
        int start_tcp_service(struct tcp_node *node)
    }
}

struct dmmr_session_connection_manager* new_session_connection_manager(struct circle_buffer* cb, struct dmmr_scheduler* sched, struct dmmr_socket *sock, struct cfg_server_server *css) {
    if (!cb || !sched)
        return 0;
    struct dmmr_session_connection_manager* mgr = (struct dmmr_session_connection_manager*)calloc(1, sizeof(struct dmmr_session_connection_manager));
    if (!mgr)
        return 0;
    mgr->reload = 0;
    cb->enqueue = session_manager_enqueue;
    sched->trigger_send = trigger_send;
    _circle_buffer = cb;
    _sched = sched;
    _sock = sock;
    _this = mgr;
    return mgr;
}
