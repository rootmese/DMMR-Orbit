#include <stdio.h>
#include <unistd.h>

#include <dmmr_session_connection_manager.h>
#include <rb_log.h>
#include <rb_trycatch.h>

static struct cfg_server_server *cfg = 0;
static struct dmmr_circle_buffer* _circle_buffer = 0;
static struct dmmr_scheduler* _sched = 0;
static struct dmmr_socket* _sock = 0;
static struct dmmr_session_connection_manager* _this = 0;

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

static inline struct session_connection_pool* session_get_session(union protocol_base_cb *u){
    return ((u) ? (get_session(u)) : (0));
}

static inline int socket_start_accept_from_uri(
    const unsigned char *uri,
    void (*on_accept_udp_cb)(struct udp_node*),
    void (*on_accept_tcp_cb)(struct tcp_node*),
    void (*on_connect_udp_cb)(struct udp_node*),
    void (*on_connect_tcp_cb)(struct tcp_node*),
	void (*on_close_udp_cb)(struct udp_node*),
	void (*on_close_tcp_cb)(struct tcp_node*)
    ) {
    return _sock->start_accept_from_uri(uri, on_accept_udp_cb, on_accept_tcp_cb, on_connect_udp_cb, on_connect_tcp_cb, on_close_udp_cb, on_close_tcp_cb);
}

static inline int socket_create_dispatcher_from_uri(
    const unsigned char *uri,
    void (*on_accept_udp_cb)(struct udp_node*),
    void (*on_accept_tcp_cb)(struct tcp_node*),
    void (*on_connect_udp_cb)(struct udp_node*),
    void (*on_connect_tcp_cb)(struct tcp_node*),
	void (*on_close_udp_cb)(struct udp_node*),
	void (*on_close_tcp_cb)(struct tcp_node*)
    ) {
    return _sock->create_dispatcher_from_uri(uri, on_accept_udp_cb, on_accept_tcp_cb, on_connect_udp_cb, on_connect_tcp_cb, on_close_udp_cb, on_close_tcp_cb);
}

static inline int sched_insert(struct session_connection_pool *pool) {
    return (pool ? _sched->insert(pool) : EOF);
}

static inline void sched_delete(struct session_connection_pool *pool) {
    _sched->delete(pool);
}

static inline void sm_delete_session(union protocol_base_cb *u){
    if(u){
        struct session_connection_pool *s = get_session(u);
        if(s){
            sched_delete(s);
            delete_session(s, !0);
        }
    }
}

struct dmmr_session_connection_manager* new_session_connection_manager(
    struct dmmr_circle_buffer* cb,
    struct dmmr_scheduler* sched,
    struct dmmr_socket* sock,
    struct cfg_server_server* css)
{
    try_catch_t ctx;
    RB_TRY(ctx) {
        // Validação dos parâmetros
        if (!cb || !sched || !sock)
            RB_THROW(ctx, RB_EXC_NULL_CONFIG);

        // Alocação do gerenciador
        struct dmmr_session_connection_manager* mgr = 
            (struct dmmr_session_connection_manager*)calloc(1, sizeof(struct dmmr_session_connection_manager));
        if (!mgr)
            RB_THROW(ctx, RB_EXC_ALLOC_FAIL);

        // Configuração básica
        cfg = css;
        _circle_buffer = cb;
        _sched = sched;
        _sock = sock;
        _this = mgr;

        // Inicialização das funções
        mgr->trunk = trunk;
        mgr->connect = socket_connect;
        mgr->close = socket_close;
        mgr->sm_delete_session = sm_delete_session;
        mgr->socket_start_accept_from_uri = socket_start_accept_from_uri;
        mgr->socket_create_dispatcher_from_uri = socket_create_dispatcher_from_uri;
        mgr->insert_session = session_insert_session;
        mgr->delete_session = session_delete_session;
        mgr->get_session = session_get_session;
        mgr->insert_scheduler = sched_insert;
        mgr->delete_scheduler = sched_delete;

        // Inicia a conexão de sessão
        int ret = start_session_connection(_circle_buffer);
        if (ret) {
            rb_log_push(LOG_ERR, "Falha ao iniciar sessões", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        // Configura callback de envio
        set_snd_cb(_sock->dispatcher);

        rb_log_push(LOG_INFO, "Gerenciador de sessões criado", __func__, __LINE__);
        return mgr;
    }
    RB_CATCH(ctx) {
        // Rollback em caso de falha
        if (_this) {
            stop_session_connection();
            free(_this);
            _this = 0;
        }
        rb_log_push(LOG_ERR, rb_exc_message(ctx.exception_code), __func__, __LINE__);
        return 0;
    }
}

void delete_session_connection_manager(void){
    stop_session_connection();
}
