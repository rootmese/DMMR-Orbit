#include <dmmr_scheduler.h>


static size_t scheduler_connection_size = 0;
static size_t scheduler_connection_count = 0;
static struct scheduler_connection *slots; // neste buffer são armazedos os pools preemptivos
static pthread_mutex_t slots_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint16_t run = 0;
  
struct dmmr_scheduler *this = 0;

static uint64_t _get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (tv.tv_sec * 1000000ULL) + tv.tv_usec;
}

static uint64_t _get_monotonic_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000ULL) + (ts.tv_nsec / 1000);
}

// TODO, outra thread ficará constantemente procurando novas conexões para inserir em slots e realocar caso seja necessário, eu queria evitar mutex, porém creio que compensa
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

static void* _reorder(void* arg) {
    struct dmmr_scheduler* s = (struct dmmr_scheduler*)arg;
    do {
        pthread_mutex_lock(&slots_mutex);
        for (int i = 1; i < scheduler_connection_count; i++) {
            struct scheduler_connection key = *(slots + i);
            uint64_t key_deadline = key.realtime_deadline_us;
            int left = 0;
            int right = i - 1;
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
        // TODO validar estender o lock para a rotina abaixo
        uint64_t now = _get_monotonic_time_us();
        for (int i = 0; i < scheduler_connection_count; i++) {
            struct scheduler_connection* conn = slots + i;
            if (now + SCHEDULER_DEADLINE_TOLERANCE_US >= conn->realtime_deadline_us) {
                s->trigger_send(conn);
            } else {
                break;
            }
        }
        struct timespec next;
        clock_gettime(CLOCK_MONOTONIC, &next);
        next.tv_nsec += 9000; // 9 µs em nanosegundos, torcar para tempo_real confugurado / 2
        if (next.tv_nsec >= 1000000000) {
            next.tv_sec++;
            next.tv_nsec -= 1000000000;
        }
        pthread_mutex_unlock(&slots_mutex);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    } while(!0);
}

static void trigger_send(scheduler_connection* conn) {
    unsigned siz = 0;
    if(!conn)
        return;
    struct session_connection_pool* pool = conn->session_ptr;
    union protocol_base_cb *u = (&(pool->session));

    if (!pool || !u)
        return;
    unsigned count = 0;
    struct node *n0 = pool->poll, *n1 = n0 + pool->pool_count;
    do{
        ++count;
    }while(++n0 < n1);
    this->sock->dispatcher(&(pool->session), n, count);
    pthread_mutex_lock(&pool->mutex);
    pool->pool_count = 0;
    memset(pool->pool, 0, pool->pool_size * sizeof(struct node));
    conn->last_active_time_us = 0;
    conn->realtime_deadline_us = 0;
    conn->deadline_us = 0;
    pthread_mutex_unlock(&pool->mutex);
}


static void send(scheduler_connection* conn, struct node *n, unsigned s) {
    unsigned siz = 0;
    if(!conn)
        return;
    struct session_connection_pool* pool = conn->session_ptr;
    union protocol_base_cb *u = (&(pool->session));

    if (!pool || !u)
        return;

    siz = ((pool->pool_size < 6) ? (pool->pool_size) : (6));
    struct node *n0 = pool->pool, *n1 = n0 + siz;
    do{
        this->sock->dispatcher(u, n0, s);
    }while(++n0 < n1);
    pthread_mutex_lock(&slots_mutex);
    __bcpy(pool + siz, pool, (scheduler_connection_count - siz - 1)); 
    conn->last_active_time_us = n0->arrival;
    conn->realtime_deadline_us = n0->deadline;
    conn->deadline_us = n0->deadline;
    pthread_mutex_unlock(&slots_mutex); //TODO tocar o mute para cada sessão ter seu mutex
}

static void reload(struct cfg_server_server* __cfg_server){
}

static int start(struct dmmr_scheduler *this){
    if(this){
        slots = (struct scheduler_connection*)calloc(0x400, sizeof(struct scheduler_connection));
        if(!slots)
            return EOF;
        scheduler_connection_size = 0x400;
        pthread_mutex_init(&slots_mutex, 0);
        pthread_create(&this->worker_thread, 0, _reorder, this);
        return 0;
    }
    else
        return EOF;
}

static void stop(struct dmmr_scheduler *this){
    pthread_cancel(this->worker_thread);
    pthread_join(this->worker_thread, 0);

}

static int insert(struct dmmr_scheduler *this, struct session_connection_pool *pool){
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

static void delete(struct session_connection_pool *pool){
    struct scheduler_connection *p0 = slots, *p1 = p0 + scheduler_connection_count;
    for (; p0 < p1; ++p0)
            if (p0->session_ptr == pool){
                pthread_mutex_lock(&slots_mutex);
                __bcpy(p0 + 1, p0, (p1 - (p0 + 1)) * sizeof(struct scheduler_connection));
                --scheduler_connection_count;
                pthread_mutex_unlock(&slots_mutex);
                break;
            }
}

struct dmmr_scheduler* new_dmmr_scheduler(struct dmmr_socket *sock, struct cfg_server_server *__cfg_server)
{
    struct dmmr_scheduler *p = (struct dmmr_scheduler*)calloc(1, sizeof(struct dmmr_scheduler));
    if(p)
    {
        p->sock = sock;
        p->deadline = __cfg_server->scheduler_preemptive_deadline;
        p->reload = reload;
        p->start = start;
        p->stop = stop;
        p->insert = insert;
        p->delete = delete;
        this = p;
        return p;
    }
    else
        return 0;
}
