#include <stdio.h>
#include <unistd.h>

#include <__mset.h>
#include <__bcpy.h>
#include <__vcpy.h>
#include <__node_cmp.h>
#include <get_session_node.h>
#include <update_session_counter.h>
#include <dmmr_sleep.h>

#include <session_connection.h>

struct session_connection_pool_recno{
    uint16_t port;
    struct session_connection_pool *pool;
};

static pthread_mutex_t session_connection_pool_mutex;

static struct dmmr_circle_buffer *circle_buffer = 0;

static unsigned session_connection_pool_recno_count = 0;

static unsigned session_connection_pool_recno_size = 0;

static struct session_connection_pool_recno *recno = 0;

static void (*snd_cb)(union protocol_base_cb *session, struct node*, unsigned size) = 0;

struct session_connection_pool *get_recno_slot(void) {
    register struct session_connection_pool_recno *p = recno;
    register struct session_connection_pool_recno *p1 = p + session_connection_pool_recno_count;
    pthread_mutex_lock(&session_connection_pool_mutex);
    for (; p < p1; ++p)
        if(!(p->pool && p->pool->run)){
            pthread_mutex_unlock(&session_connection_pool_mutex);
            return p->pool;
        }
    if (session_connection_pool_recno_count >= session_connection_pool_recno_size) {
        session_connection_pool_recno_size *= 2;
        recno = (struct session_connection_pool_recno*)realloc(recno, session_connection_pool_recno_size * sizeof(struct session_connection_pool_recno));
        if (!recno){
            pthread_mutex_unlock(&session_connection_pool_mutex);
            return 0;
        }
    }
    struct session_connection_pool *ret = (recno + session_connection_pool_recno_count++)->pool;
    pthread_mutex_unlock(&session_connection_pool_mutex);
    return ret;
}

void delete_recno_slot(struct session_connection_pool *p){
    if(p){
    pthread_mutex_lock(&session_connection_pool_mutex);
    pthread_mutex_destroy(&(p->mutex));
    __mset(p, 0, sizeof(struct session_connection_pool));
    pthread_mutex_unlock(&session_connection_pool_mutex);
    }
}

static inline void sort_pool_by_arrival_inline(struct node *v, size_t c) {
    if (!v || c < 2)
        return;

    for (size_t i = 1; i < c; ++i) {
        struct node key = *(v + i);
        size_t j = i;
        while (j > 0 && (v + j - 1)->arrival > key.arrival) {
            *(v + j) = *(v + j - 1);
            --j;
        }
        *(v + j) = key;
    }
}

static inline uint16_t get_session_size(struct session_connection_pool *p, uint32_t max) {
    if(p){
        struct node *n = p->pool, *n1 = n + p->pool_count;
        uint16_t total = 0;
        if(p->pool_count)
            do {
                total += n->value_size;
                if(total >=max)
                    return total;
            } while(++n < n1);
        return total;
    }
    else
        return 0;
}

static inline void process_session_node(struct session_connection_pool *conn) {
    if (!(node_cmp(&(conn->cursor->n), get_session_node(&(conn->session))))) {
        *(conn->pool + conn->pool_count) = conn->cursor->n;
        conn->pool_count++;
        uint16_t size = get_session_size(conn, 8520); // TODO valor baseado em 1500 de MTU
        if (size >= 8520 && conn->pool_size > 0) { // TODO valor baseado em 1500 de MTU
            unsigned count = 0, siz = 0;
            struct node *n = conn->pool, *n0 = n + conn->pool_count;
            do {
                siz += n->value_size;
                ++count;
            } while (siz <= 8520 && ++n < n0);
            if (snd_cb)
                snd_cb(&(conn->session), conn->pool, count);
            if (count <= conn->pool_count) {
                pthread_mutex_lock(&conn->mutex);
                __bcpy(conn->pool + count, conn->pool, (conn->pool_count - count) * sizeof(struct node));
                conn->pool_count -= count;
                update_session_counter(&(conn->session));
                sort_pool_by_arrival_inline(conn->pool, conn->pool_count);
                pthread_mutex_unlock(&conn->mutex);
            }
            else
                conn->pool_count = 0;
        }
    }
}

static void* session_worker(void* arg) {
    struct session_connection_pool* conn = (struct session_connection_pool*)arg;
    if (!conn || !conn->pool || !circle_buffer)
        return 0;
    struct circleq_head *head = &circle_buffer->head;
    do {
        if (conn->pool_count + 1 > conn->pool_size) {
            pthread_mutex_lock(&conn->mutex);
            conn->pool_size *= 2;
            conn->pool = (struct node*)realloc(conn->pool, conn->pool_size * sizeof(struct node));
            if (!(conn->pool)) {
                pthread_mutex_unlock(&conn->mutex);
                goto return_fail;
            }
            pthread_mutex_unlock(&conn->mutex);
        }
        process_session_node(conn);
        while (!(circle_buffer->is_behind_cursor(conn->cursor)))
            NSLEEP_US(10);
        conn->cursor = circle_buffer->iterate(conn->cursor, head);
        NSLEEP_US(10);
    } while (conn->run);

return_fail:
    return 0;
}

void set_snd_cb(void (*send_cb)(union protocol_base_cb *session, struct node*, unsigned size)){
    snd_cb = send_cb;
}

int insert_session(struct session_connection_pool *session) {
    int ret;
    if(!circle_buffer || !session)
        return EOF;
    session->pool = (struct node*)calloc(0x400, sizeof(struct node));
    if(!(session->pool))
        return EOF;
    session->pool_size = 0x400;
    session->pool_count = 0;
    session->run = !0;
    session->cursor = circle_buffer->get_current_node();
    pthread_mutex_init(&session->mutex, NULL);
    spawn_detached_thread(&session->thread, session_worker, session, &ret);
    return ret;
}

void delete_session(struct session_connection_pool *p, int do_sleep){
    if (p) {
        pthread_mutex_t m;
        __vcpy(&m,&(p->mutex), sizeof(pthread_mutex_t));
        p->run = 0;
        if (do_sleep)
            sleep(1);
        if (p->pool)
            free(p->pool);
        __mset(p, 0, sizeof(struct session_connection_pool));
        pthread_mutex_destroy(&m);
    }
}

int reload_session_conection(void){
    return 0;
}

int start_session_connection(struct dmmr_circle_buffer *__cb){
    if(recno)
        return EOF;
    else{
        recno = (struct session_connection_pool_recno*)calloc(0x400, sizeof(struct session_connection_pool_recno));
        if(!recno)
            return EOF;
        session_connection_pool_recno_size = 0x400;
        circle_buffer = __cb;
        pthread_mutex_init(&session_connection_pool_mutex, 0);
        return 0;
    }
}

void stop_session_connection(void){
    if(recno){
        struct session_connection_pool_recno *p = recno;
        struct session_connection_pool_recno *p1 = p + session_connection_pool_recno_count;
        for (; p < p1; ++p)
            if(!(p->pool && p->pool->run))
                delete_session(p->pool, 0);
        sleep(1);
        free(recno);
        recno = 0;
        session_connection_pool_recno_size = 0;
        session_connection_pool_recno_count = 0;
        pthread_mutex_destroy(&session_connection_pool_mutex);
    }
}