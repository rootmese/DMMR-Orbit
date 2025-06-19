#include <stdio.h>
#include <unistd.h>

#include <__mset.h>
#include <parse_uri.h>

#include <dmmr_session_connection_manager.h>

static struct cfg_server_server *cfg = 0;
static struct dmmr_circle_buffer* _circle_buffer = 0;
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
                return _sock->start_acception(proto, port, (const char*)host_buf);
        }
    }
    return EOF;
}

static int socket_connect(const unsigned char *uri) {
    // TODO: Implement connect logic
    return EOF;
}

static void trunk(const unsigned char *accept_uri, const unsigned char *connect_uri) {
     // TODO: Implement trunk logic
}

static void socket_close(const unsigned char *uri) {
    // TODO: implement close
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
    _sock->set_acception_cb_tcp = cb;
}

static inline void socket_set_dispatch_cb_udp(void (*cb)(struct udp_node*)) {
    _sock->set_acception_cb_udp = cb;
}

static inline void socket_set_dispatch_cb_tcp(void (*cb)(struct tcp_node*)) {
    _sock->set_acception_cb_tcp = cb;
}


static inline int socket_start_accept_from_uri(const unsigned char *uri) {
    return _sock->start_accept_from_uri(uri);
}

static inline int socket_create_dispatcher_from_uri(const unsigned char *uri) {
    return _sock->create_dispatcher_from_uri(uri); // Se for outro método, ajustar nome
}

static inline int sched_insert(struct session_connection_pool *pool) {
    return (pool ? _sched->insert(pool) : EOF);
}

static inline void sched_delete(struct session_connection_pool *pool) {
    _sched->delete(pool);
}

struct dmmr_session_connection_manager* new_session_connection_manager(
    struct dmmr_circle_buffer* cb,
    struct dmmr_scheduler* sched,
    struct dmmr_socket* sock,
    struct cfg_server_server* css)
{
    if (!cb || !sched || !sock)
        return 0;
    int ret;
    struct dmmr_session_connection_manager* mgr = (struct dmmr_session_connection_manager*)calloc(1, sizeof(struct dmmr_session_connection_manager));
    if (!mgr)
        return 0;

    cfg = css;

    mgr->reload                               = 0;
    mgr->trunk                                = trunk; // TODO future
    mgr->accept                               = socket_accept;
    mgr->connect                              = socket_connect;
    mgr->close                                = socket_close;
    mgr->socket_start_accept_from_uri         = socket_start_accept_from_uri;
    mgr->socket_create_dispatcher_from_uri    = socket_create_dispatcher_from_uri;
    mgr->set_socket_acception_cb_udp          = socket_set_acception_cb_udp;
    mgr->set_socket_acception_cb_tcp          = socket_set_acception_cb_tcp;
    mgr->set_socket_dispatch_cb_udp           = socket_set_dispatch_cb_udp;
    mgr->set_socket_dispatch_cb_tcp           = socket_set_dispatch_cb_tcp;
    mgr->insert_session                       = session_insert_session;
    mgr->delete_session                       = session_delete_session;
    mgr->insert_scheduler                     = sched_insert;
    mgr->delete_scheduler                     = sched_delete;

    _circle_buffer = cb;
    _sched = sched;
    _sock = sock;
    _this = mgr;

    ret = start_session_connection(_circle_buffer);
    if(ret)
        return 0;

    set_snd_cb(_sock->dispatcher);

    return mgr;
}

void delete_session_connection_manager(void){
    stop_session_connection();
}
