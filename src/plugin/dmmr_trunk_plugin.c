#include <stdio.h>
#include <dmmr_plugin.h>

#include <__vcpy.h>

static struct dmmr_session_connection_manager *sm = 0;
static struct cfg_server_server *cfg = 0;
static struct dmmr_plugin *this = 0;

static void on_dispatch_connection_udp_cb(struct udp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                sm->insert_scheduler(p);
            }
        }
    }
}

static void on_dispatch_connection_tcp_cb(struct tcp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                sm->insert_scheduler(p);
            }
        }
    }
}

static void on_acception_connection_udp_cb(struct udp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                ret = sm->insert_scheduler(p);
                if (!ret) {
                    sm->set_socket_dispatch_cb_udp(on_dispatch_connection_udp_cb);
                    sm->set_socket_dispatch_cb_tcp(on_dispatch_connection_tcp_cb);
                    sm->create_dispatcher_from_uri(cfg->trunk_dispatch_uri);
                }
            }
        }
    }
}

static void on_acception_connection_tcp_cb(struct tcp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot();
        if (p) {
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            ret = sm->insert_session(p);
            if (!ret) {
                ret = sm->insert_scheduler(p);
                if (!ret) {
                    sm->set_socket_dispatch_cb_udp(on_dispatch_connection_udp_cb);
                    sm->set_socket_dispatch_cb_tcp(on_dispatch_connection_tcp_cb);
                    sm->create_dispatcher_from_uri(cfg->trunk_dispatch_uri);
                }
            }
        }
    }
}

static void dmmr_plugin_reload(void){
}

static void dmmr_plugin_load(void) {
    if (sm && cfg) {
        sm->set_socket_acception_cb_udp(on_acception_connection_udp_cb);
        sm->set_socket_acception_cb_tcp(on_acception_connection_tcp_cb);
        (void)sm->socket_start_accept_from_uri(cfg->trunk_accept_uri);
    }
}

struct dmmr_plugin *new_dmmr_plugin(struct dmmr_session_connection_manager *__sm, struct cfg_server_server *__cfg) {
    if (__sm && __cfg) {
        this = (struct dmmr_plugin *)calloc(1, sizeof(struct dmmr_plugin));
        if (this) {
            sm = __sm;
            cfg = __cfg;
            this->load   = dmmr_plugin_load;
            this->reload = dmmr_plugin_reload;
        }
    }
    return this;
}
