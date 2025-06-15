#include <defs.h>

static struct circle_buffer* _circle_buffer = 0;
static struct dmmr_scheduler* _sched = 0;
static struct dmmr_socket* _sock = 0;
static struct dmmr_session_connection_manager* _this = 0;

static int socket_accept(const unsigned char *uri) {
    if (uri) {
        uint16_t port;
        unsigned char host_buf[0x100];
        __mset(host_buf, 0, sizeof(host_buf));
        proto_t proto = parse_protocol_host_port(uri, host_buf, sizeof(host_buf), &port);
        switch (proto) {
            case proto_none_t:
                return EOF;
            case proto_udp_t:
            case proto_tcp_t:
                return start_acception(proto, port, (const char*)host_buf);
        }
    }
    return EOF;
}

static int socket_connect(const unsigned char *uri) {
    // TODO: Implement connect logic
    return EOF;
}

static void trunk(const unsigned char *accept_uri, const unsigned char *connect_uri) {
    if (accept_uri) {
        int ret = socket_accept(accept_uri);
        if (ret != 0) {
            // log or handle error
        }
    }
    // TODO: handle connect_uri
}

static void socket_close(const unsigned char *uri) {
    // TODO: implement close
}

static int socket_send(const uint8_t *output, uint32_t output_size) {
    if (_sock && output && output_size > 0) {
        return _sock->send(output, output_size); // delega pra dmmr_socket
    }
    return EOF;
}

static inline int session_insert_session(struct session_connection_pool *session) {
    return (session ? insert_session(session) : EOF);
}

static inline void session_delete_session(struct session_connection_pool *session) {
    if (session) {
        // TODO: delete logic
    }
}

static inline void socket_set_acception_cb_udp(void (*cb)(struct udp_node*)) {
    _sock->set_acception_cb_udp = cb;
}

static inline void socket_set_acception_cb_tcp(void (*cb)(struct tcp_node*)) {
    _sock->set_socket_acception_cb_tcp = cb;
}

static inline int socket_start_accept_from_uri(const unsigned char *uri) {
    return _sock->start_accept_from_uri(uri);
}

static inline int socket_create_dispatcher_from_uri(const unsigned char *uri) {
    return _sock->start_dispatcher_from_uri(uri); // Se for outro método, ajustar nome
}

static inline int sched_insert(struct session_connection_pool *pool) {
    return (pool ? _sched->insert(pool) : EOF);
}

struct dmmr_session_connection_manager* new_session_connection_manager(
    struct circle_buffer* cb,
    struct dmmr_scheduler* sched,
    struct dmmr_socket* sock,
    struct cfg_server_server* css)
{
    if (!cb || !sched || !sock)
        return 0;

    struct dmmr_session_connection_manager* mgr = (struct dmmr_session_connection_manager*)calloc(1, sizeof(struct dmmr_session_connection_manager));
    if (!mgr)
        return 0;

    mgr->reload                        = 0;
    mgr->accept                        = socket_accept;
    mgr->connect                       = socket_connect;
    mgr->trunk                         = trunk;
    mgr->close                         = socket_close;
    mgr->send                          = socket_send;
    mgr->start_accept_from_uri        = socket_start_accept_from_uri;
    mgr->create_dispatcher_from_uri   = socket_create_dispatcher_from_uri;
    mgr->set_socket_acception_cb_udp  = socket_set_acception_cb_udp;
    mgr->set_socket_acception_cb_tcp  = socket_set_acception_cb_tcp;
    mgr->insert_session               = session_insert_session;
    mgr->delete_session               = session_delete_session;
    mgr->insert_scheduler             = sched_insert;

    _circle_buffer = cb;
    _sched = sched;
    _sock = sock;
    _this = mgr;

    return mgr;
}
