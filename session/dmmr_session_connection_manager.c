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

static void trigger_send(struct dmmr_scheduler *sched, struct scheduler_connection *sched_conn){
    if(sched && sched_conn){
        struct node_circle_buffer *s = get_session(sched_conn->session_ptr->port);
        if(s)
            session_connection_trigger_send(sched_conn);
    }
}

static void acception_connection_tcp_cb(struct tcp_node *input){
    if(input){
        struct session_connection_pool *p = get_recno_slot();
        i(p){
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            _sched->insert(_sched, p);
        }
    }
}

struct dmmr_session_connection_manager* new_session_connection_manager(struct circle_buffer* cb, struct dmmr_scheduler* sched, struct dmmr_socket *sock, struct cfg_server_server *css) {
    if (!cb || !sched)
        return 0;
    struct dmmr_session_connection_manager* mgr = calloc(1, sizeof(struct dmmr_session_connection_manager));
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
