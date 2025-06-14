#include <defs.h>

static struct circle_buffer* _circle_buffer = 0;
static struct dmmr_scheduler* _sched = 0;
static struct dmmr_socket *_sock = 0;
static struct dmmr_session_connection_manager _this = 0;

static void (*on_accept_cb)(struct node*) = 0;
static void (*on_dispatch_cb)(struct node*) = 0;

static int accept(const char *uri){
    if(uri){
        uint16_t port;
        char host_buf[0x100];
        proto_t proto = parse_protocol_host_port(uri, host_buf, sizeof(host_buf), &port);
        switch(proto){
            case proto_none_t:
                return EOF;
                break; /* Stupid break */
            case proto_udp_t:
            case proto_tcp_t:
                return start_acception(proto, port, const char* ip);
                break;
        }
    }
    return EOF;
}

static int connect(const char *uri){

}

static void trunk(const unsigned char *accept_uri, const char *connect_uri){

}

void close(const unsigned char *uri){

}

static int send(const uint8_t *output, uint32_t output_size){

}

static void set_receiver_cb(uint8_t input, uint32_t input_size){

}

static void set_accept_cb(const char *conn_str){

}

struct dmmr_session_connection_manager* new_session_connection_manager(struct circle_buffer* cb, struct dmmr_scheduler* sched, struct dmmr_socket *sock, struct cfg_server_server *css) {
    if (!cb || !sched)
        return 0;
    struct dmmr_session_connection_manager* mgr = (struct dmmr_session_connection_manager*)calloc(1, sizeof(struct dmmr_session_connection_manager));
    if (!mgr)
        return 0;
    mgr->reload = 0;
    mgr->accept = accept;
    mgr->connect = connect;
    mgr->trunk = trunk;
    mgr->close = close;
    mgr->send = send;
    _circle_buffer = cb;
    _sched = sched;
    _sock = sock;
    _this = mgr;
    return mgr;
}
