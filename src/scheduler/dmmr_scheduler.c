#include <stdio.h>

#include <dmmr_sleep.h>
#include <__bcpy.h>
#include <__vcpy.h>
#include <__mset.h>

#include <dmmr_scheduler.h>
#include <dmmr_parser.h>

static size_t scheduler_connection_size = 0;
static size_t scheduler_connection_count = 0;
static struct scheduler_connection *slots;
static pthread_mutex_t slots_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint8_t run = 0;

static struct cfg_server_server *cfg = 0;
  
struct dmmr_scheduler *this = 0;

static inline  uint64_t _get_monotonic_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000ULL) + (ts.tv_nsec / 1000);
}

static int ensure_capacity(void) {
    if (scheduler_connection_count >= scheduler_connection_size)
    {
        pthread_mutex_lock(&slots_mutex);
        scheduler_connection_size *= 2;
        struct scheduler_connection *new_slots = realloc(slots, scheduler_connection_size * sizeof(struct scheduler_connection));
        if (!new_slots)
        {
            pthread_mutex_unlock(&slots_mutex);
            return EOF;
        }
        slots = new_slots;
        pthread_mutex_unlock(&slots_mutex);
        return 0;
    }
    else
        return 0;
}

static void* _reorder_thread(void* arg) {
    (void)arg;
    do {
        pthread_mutex_lock(&slots_mutex);
        for (size_t i = 1; i < scheduler_connection_count; i++) {
            struct scheduler_connection key = *(slots + i);
            uint64_t key_deadline = key.realtime_deadline_us;
            int left = 0, right = i - 1;
            while (left <= right) {
                int mid = (left + right) / 2;
                if ((*(slots + mid)).realtime_deadline_us <= key_deadline)
                    left = mid + 1;
                else
                    right = mid - 1;
            }
            for (int j = i - 1; j >= left; j--) {
                *(slots + j + 1) = *(slots + j);
            }
            *(slots + left) = key;
        }
        pthread_mutex_unlock(&slots_mutex);
        NSLEEP_US(cfg->sleep_time);
    }while (run);
    return 0;
}


static void* _check_and_send_thread(void* arg) {
    register uint64_t now;
    register struct scheduler_connection *s0, *s1;
    struct dmmr_scheduler* s = (struct dmmr_scheduler*)arg;
     do{
        now = _get_monotonic_time_us();
        if(scheduler_connection_count){
            pthread_mutex_lock(&slots_mutex);
            s0 = slots, s1 = slots + scheduler_connection_count;
            do {
                if (now + cfg->scheduler_preemptive_deadline >= s0->realtime_deadline_us)
                    s->trigger_send(s0);
                else
                    break; // slots estão ordenados!
            }while(++s0 < s1);
            pthread_mutex_unlock(&slots_mutex);
        }
    }while (run);
    NSLEEP_US(cfg->sleep_time);
    return 0;
}

static void sched_trigger_snd(struct scheduler_connection* conn) {
    if(!conn)
        return;
    struct session_connection_pool* pool = conn->session_ptr;
    union protocol_base_cb *u = (&(pool->session));
    if (!pool || !u)
        return;
    pthread_mutex_lock(&pool->mutex);
    unsigned count = 0;
    struct node *n0 = pool->pool, *n1 = n0 + pool->pool_count;
    do{
        ++count;
    }while(++n0 < n1);
    this->sock->dispatcher(&(pool->session), pool->pool, count);
    pool->pool_count = 0;
    __mset(pool->pool, 0, pool->pool_size * sizeof(struct node));
    pthread_mutex_unlock(&pool->mutex);
    conn->last_active_time_us = 0;
    conn->realtime_deadline_us = 0;
    conn->deadline_us = 0;
}


static void sched_snd(struct scheduler_connection* conn, struct node *n, unsigned s) {
    if(!conn)
        return;
    unsigned siz = 0;
    struct session_connection_pool* pool = conn->session_ptr;
    union protocol_base_cb *u = (&(pool->session));
    if (!pool || !u)
        return;
    pthread_mutex_lock(&(pool->mutex));
    siz = ((pool->pool_size <= 6) ? (pool->pool_size) : (6));
    struct node *n0 = n, *n1 = n0 + siz;
    do{
        this->sock->dispatcher(u, n0, s);
    }while(++n0 < n1);
    __bcpy(n + siz, n, (pool->pool_size - siz) * sizeof(struct node));
    pthread_mutex_lock(&slots_mutex);
    conn->last_active_time_us = n->arrival;
    conn->realtime_deadline_us = n->deadline;
    conn->deadline_us = n->deadline;
    pool->pool_count -= siz;
    pool->pool_size -= siz;
    pthread_mutex_unlock(&slots_mutex);
    pthread_mutex_unlock(&(pool->mutex));
    
}

static void sched_reload(void){
}

static int sched_start(void){
    if(this){
        int ret;
        pthread_attr_t attr;
        struct sched_param param;
        slots = (struct scheduler_connection*)calloc(0x400, sizeof(struct scheduler_connection));
        if(!slots)
            return EOF;
        scheduler_connection_size = 0x400;
        run = !0;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);  // Escalonador em tempo real
        param.sched_priority = 80;  // Prioridade entre 1 e 255 (quanto maior, mais prioridade)
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);  // Aplica a prioridade explicitamente
        pthread_mutex_init(&slots_mutex, 0);
        spawn_detached_thread(&this->reorder_thread, _reorder_thread, this, &ret);
        if(ret)
            return ret;
        spawn_detached_thread_with_attr(&this->send_thread, &attr, _check_and_send_thread, this, &ret);
        return 0;
    }
    else
        return EOF;
}

static void sched_stop(void){
    run = 0;
    sleep(1);
    if(slots)
        free(slots);
}

static int sched_insert(struct session_connection_pool *pool){
    int ret = ensure_capacity();
    if(ret)
        return EOF;
    else{
        pthread_mutex_lock(&slots_mutex);
        struct scheduler_connection *p = slots + scheduler_connection_count++;
        p->session_ptr = pool;
        p->last_active_time_us = pool->pool->arrival;
        p->realtime_deadline_us = pool->pool->deadline;
        p->deadline_us = this->deadline;
        pthread_mutex_unlock(&slots_mutex);
        return 0;
    }
}

static void sched_delete(struct session_connection_pool *pool){
    pthread_mutex_lock(&slots_mutex);
    struct scheduler_connection *p0 = slots, *p1 = p0 + scheduler_connection_count;
    for (; p0 < p1; ++p0)
            if (p0->session_ptr == pool){
                __bcpy(p0 + 1, p0, (p1 - (p0 + 1)) * sizeof(struct scheduler_connection));
                --scheduler_connection_count;
                break;
            }
    pthread_mutex_unlock(&slots_mutex);
}

struct dmmr_scheduler* new_dmmr_scheduler(struct dmmr_socket *__sock, struct cfg_server_server *__cfg_server){
    struct dmmr_scheduler *p = (struct dmmr_scheduler*)calloc(1, sizeof(struct dmmr_scheduler));
    if(p)
    {
        cfg = __cfg_server;
        p->sock = __sock;
        p->reload = sched_reload;
        p->start = sched_start;
        p->stop = sched_stop;
        p->insert = sched_insert;
        p->delete = sched_delete;
        p->send = sched_snd;
        p->trigger_send = sched_trigger_snd;
        this = p;
        return p;
    }
    else
        return 0;
}
