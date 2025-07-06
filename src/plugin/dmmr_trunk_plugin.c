
#include <dmmr_plugin.h>

static struct dmmr_session_connection_manager *sm = 0;
static struct cfg_server_server *cfg = 0;
static struct dmmr_plugin *this = 0;

// -- Callbacks de encerramento --

static void on_close_connection_udp_cb(struct udp_node *input) {
    sm->sm_delete_session((union protocol_base_cb*)input);
}

static void on_close_connection_tcp_cb(struct tcp_node *input) {
    if (sm)
        sm->sm_delete_session((union protocol_base_cb*)input);
}

// -- Callbacks de dispatch (não usados diretamente ainda) --

static void on_dispatch_connection_udp_cb(struct udp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot(input->node->port, proto_arr_udp_t);
        if (p) {
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            input->linked = &(p->session);
            p->session.udp.linked = (union protocol_base_cb*)input;
            p->session.udp.proto = proto_udp_t;
            link_session_ports(p, &p->session);
            ret = sm->insert_session(p);
            if (!ret)
                sm->insert_scheduler(p);
        }
    }
}

static void on_dispatch_connection_tcp_cb(struct tcp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot(input->node->port, proto_arr_tcp_t);
        if (p) {
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            input->linked = &(p->session);
            p->session.tcp.linked = (union protocol_base_cb*)input;
            p->session.tcp.proto = proto_tcp_t;
            link_session_ports(p, &p->session);
            ret = sm->insert_session(p);
            if (!ret)
                sm->insert_scheduler(p);
        }
    }
}

// -- Callbacks de conexão ativa --

static void on_connect_connection_udp_cb(struct udp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot(input->node->port, proto_arr_udp_t);
        if (p) {
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            input->linked = &(p->session);
            p->session.udp.linked = (union protocol_base_cb*)input;
            p->session.udp.proto = proto_udp_t;
            link_session_ports(p, &p->session);
            ret = sm->insert_session(p);
            if (!ret)
                sm->insert_scheduler(p);
        }
    }
}

static void on_connect_connection_tcp_cb(struct tcp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot(input->node->port, proto_arr_tcp_t);
        if (p) {
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            input->linked = &(p->session);
            p->session.tcp.linked = (union protocol_base_cb*)input;
            p->session.tcp.proto = proto_tcp_t;
            link_session_ports(p, &p->session);
            ret = sm->insert_session(p);
            if (!ret)
                sm->insert_scheduler(p);
        }
    }
}

// -- Callbacks de aceitação passiva --

static void on_acception_connection_udp_cb(struct udp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot(input->node->port, proto_arr_udp_t);
        if (p) {
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            p->session.udp.linked = (union protocol_base_cb*)input;
            p->session.udp.proto = proto_udp_t;
            link_session_ports(p, &p->session);
            ret = sm->insert_session(p);
            if (!ret) {
                ret = sm->insert_scheduler(p);
                if (!ret) {
                    sm->socket_create_dispatcher_from_uri(
                        cfg->trunk_dispatch_uri,
                        on_dispatch_connection_udp_cb,
                        on_dispatch_connection_tcp_cb,
                        on_connect_connection_udp_cb,
                        on_connect_connection_tcp_cb,
                        on_close_connection_udp_cb,
                        on_close_connection_tcp_cb
                    );
                }
            }
        }
    }
}

static void on_acception_connection_tcp_cb(struct tcp_node *input) {
    int ret;
    if (input) {
        struct session_connection_pool *p = get_recno_slot(input->node->port, proto_arr_tcp_t);
        if (p) {
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            p->session.tcp.linked = (union protocol_base_cb*)input;
            p->session.tcp.proto = proto_tcp_t;
            link_session_ports(p, &p->session);
            ret = sm->insert_session(p);
            if (!ret) {
                ret = sm->insert_scheduler(p);
                if (!ret) {
                    sm->socket_create_dispatcher_from_uri(
                        cfg->trunk_dispatch_uri,
                        on_dispatch_connection_udp_cb,
                        on_dispatch_connection_tcp_cb,
                        on_connect_connection_udp_cb,
                        on_connect_connection_tcp_cb,
                        on_close_connection_udp_cb,
                        on_close_connection_tcp_cb
                    );
                }
            }
        }
    }
}

// -- Plugin API --

static void dmmr_plugin_reload(void) {}

static void dmmr_plugin_load(void) {
    if (sm && cfg) {
        (void)sm->socket_start_accept_from_uri(
            cfg->trunk_accept_uri,
            on_acception_connection_udp_cb,
            on_acception_connection_tcp_cb,
            on_connect_connection_udp_cb,
            on_connect_connection_tcp_cb,
            on_close_connection_udp_cb,
            on_close_connection_tcp_cb
        );
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
