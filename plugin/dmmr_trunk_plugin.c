#include <dmmr_plugin.h>

#define MAP_UDP_ACCEPT   0x01
#define MAP_UDP_CONNECT  0x02
#define MAP_TCP_ACCEPT   0x04
#define MAP_TCP_CONNECT  0x08

#define MAP_SET(flag)    (map |= (flag))
#define MAP_CLEAR(flag)  (map &= ~(flag))
#define MAP_HAS(flag)    ((map & (flag)) != 0)

static struct dmmr_session_connection_manager *sm = 0;
static struct cfg_server_server *cfg = 0;
static struct dmmr_plugin *this = 0;

static struct udp_node *udp_node[0x02];
static struct tcp_node *tcp_node[0x02];
static uint8_t map = 0x00;

static int on_dispatch_connection_udp_cb(struct udp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                (void)sm->insert_scheduler(p->session);
            }
        }
    }
    return EOF;
}

static int on_dispatch_connection_tcp_cb(struct tcp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                (void)sm->insert_scheduler(p->session);
            }
        }
    }
    return EOF;
}

static int on_acception_connection_udp_cb(struct udp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                ret = sm->insert_scheduler(p->session);
                if (!ret) {
                    sm->set_socket_dispatch_cb_udp(on_dispatch_connection_udp_cb);
                    sm->set_socket_dispatch_cb_tcp(on_dispatch_connection_tcp_cb);
                    (void)sm->create_dispatcher_from_uri(cfg->trunk_dispatch_uri);
                }
            }
        }
    }
    return EOF;
}

static int on_acception_connection_tcp_cb(struct tcp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                ret = sm->insert_scheduler(p->session);
                if (!ret) {
                    sm->set_socket_dispatch_cb_udp(on_dispatch_connection_udp_cb);
                    sm->set_socket_dispatch_cb_tcp(on_dispatch_connection_tcp_cb);
                    (void)sm->create_dispatcher_from_uri(cfg->trunk_dispatch_uri);
                }
            }
        }
    }
    return EOF;
}

static int load(void) {
    if (sm && cfg) {
        sm->set_socket_acception_cb_udp(on_acception_connection_udp_cb);
        sm->set_socket_acception_cb_tcp(on_acception_connection_tcp_cb);
        int ret = sm->socket_start_accept_from_uri(cfg->trunk_accept_uri);
        return (ret != 0) ? EOF : 0;
    }
    return EOF;
}

struct dmmr_plugin *new_dmmr_plugin(struct dmmr_session_connection_manager *__sm, struct cfg_server_server *__cfg) {
    if (__sm && __cfg) {
        this = (struct dmmr_plugin *)calloc(1, sizeof(struct dmmr_plugin));
        if (this) {
            sm = __sm;
            cfg = __cfg;
        }
    }
    return this;
}
