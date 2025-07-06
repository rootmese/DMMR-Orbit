#include <stdio.h>

#include <dmmr_circle_buffer.h>

#include <rb_log.h>

#include <rb_trycatch.h>

static struct dmmr_circle_buffer *this = 0;

static struct cfg_server_server *cfg = 0;

static struct node_circle_buffer* buffer = 0;

static int run = 0;

static inline struct node_circle_buffer* __get_current_node(void) {
    if(this){
        struct node_circle_buffer* current = this->cursor;
        return current;
    }
    else
        return 0;
}

static struct node_circle_buffer* get_current_node(void) {
    return get_current_node();
}

static inline int __is_behind_cursor(struct node_circle_buffer *cur) {
    intptr_t a = (intptr_t)cur;
    intptr_t b = (intptr_t)this->cursor;
    intptr_t base = (intptr_t)buffer;
    intptr_t end = (intptr_t)(buffer + cfg->circle_buffer_size);

    if (a == b)
        return 0; // tá em cima do cursor, não é seguro
    else if (a < b)
        return (b - a) < (end - base); // tá atrás, sem wrap
    else
        return ((end - a) + (b - base)) < (end - base); // wrap: tá atrás se a distância for < buffer
}

static inline int is_behind_cursor(struct node_circle_buffer *cur){
    if(cur)
        return __is_behind_cursor(cur);
}

static inline void foward_current_node(void){
    if(this){
        this->cursor = CIRCLEQ_NEXT(this->cursor, circleq);
        if (this->cursor == CIRCLEQ_LAST(&(this->head)))
            this->cursor = CIRCLEQ_FIRST(&(this->head));
    }
}

static void* fifo_worker(void* arg) {
    const int MAX_BATCH = 0x400;
    struct node_circle_buffer *slot = 0;
    struct node_fifo_buffer *batch[MAX_BATCH];
    struct node_fifo_buffer **b = batch;
    struct node_fifo_buffer **batch_ptr;
    struct timespec ts;

    do {
        pthread_mutex_lock(&this->fifo_lock);
        while (TAILQ_EMPTY(&this->fifo) && run) {
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 10000;  // 10μs timeout
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000;
            }
            pthread_cond_timedwait(&(this->fifo_cond), &(this->fifo_lock), &ts);
        }
        if (!run) {
            pthread_mutex_unlock(&this->fifo_lock);
            break;
        }
        while (!TAILQ_EMPTY(&this->fifo) && (b - batch) < MAX_BATCH) {
            *b = TAILQ_FIRST(&this->fifo);
            TAILQ_REMOVE(&this->fifo, *b, tailq);
            ++b;
        }
        pthread_mutex_unlock(&this->fifo_lock);
        if (b > batch) {
            batch_ptr = batch;
            do {
                slot = __get_current_node();
                if (slot && *batch_ptr)
                    __vcpy(slot, &(*batch_ptr)->n, sizeof(struct node));
                foward_current_node();
            } while (++batch_ptr < b);
        }
    } while (run);
    return 0;
}



static struct node_circle_buffer *iterate(struct node_circle_buffer *cursor, struct circleq_head *head) {
    if (!cursor || CIRCLEQ_EMPTY(head))
        return 0;
    else{
        struct node_circle_buffer *next = CIRCLEQ_NEXT(cursor, circleq);
        return (next == (struct node_circle_buffer *)CIRCLEQ_LAST(head)) ? CIRCLEQ_FIRST(head) : next;
    }
}

static int start(void){
    if(this){
        int ret;
        buffer = (struct node_circle_buffer*)calloc(cfg->circle_buffer_size, sizeof(struct node_circle_buffer));
        if (!buffer)
            return 0;

        CIRCLEQ_INIT(&(this->head));
        TAILQ_INIT(&(this->fifo)); 
        struct node_circle_buffer *prev = 0;
        struct node_circle_buffer *p = buffer, *p1 = p + cfg->circle_buffer_size;
        do
        {
            CIRCLEQ_INSERT_TAIL(&(this->head), p, circleq);
            if (prev)
                p->prev_ptr = prev;
            prev = p;
        } while (++p < p1);

        this->cursor = CIRCLEQ_EMPTY(&this->head) ? 0 : CIRCLEQ_FIRST(&this->head);
        CIRCLEQ_FIRST(&this->head)->prev_ptr = CIRCLEQ_LAST(&this->head);

        pthread_attr_t attr;
        struct sched_param param;
        run = !0;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        param.sched_priority = 80;
        pthread_attr_setschedparam(&attr, &param);

        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_mutex_init(&this->fifo_lock, 0);
        pthread_cond_init(&this->fifo_cond, 0);
        spawn_detached_thread_with_attr(&(this->fifo_thread), &attr, fifo_worker, this, &ret);

        return 0;
    }
    else
        return EOF;
}

void stop(void){
    if(this){
        run = 0;
        pthread_cond_broadcast(&this->fifo_cond);
        sleep(1);
        pthread_join(this->fifo_thread, 0);
        pthread_mutex_destroy(&this->fifo_lock);
        pthread_cond_destroy(&this->fifo_cond);
        if(buffer)
            free(buffer);
        this = 0;
    }
}

struct dmmr_circle_buffer* new_circle_buffer(struct cfg_server_server *__cfg) {
    try_catch_t ctx;
    RB_TRY(ctx) {
        if (!__cfg)
            RB_THROW(ctx, RB_EXC_NULL_CONFIG);

        struct dmmr_circle_buffer* cb = (struct dmmr_circle_buffer*)calloc(1, sizeof(struct dmmr_circle_buffer));
        if (!cb)
            RB_THROW(ctx, RB_EXC_ALLOC_FAIL);

        cfg = __cfg;
        cb->is_behind_cursor = is_behind_cursor;
        cb->get_current_node = get_current_node;
        cb->iterate = iterate;
        cb->start = start;
        cb->stop = stop;
        this = cb;

        rb_log_push(LOG_INFO, "Circle buffer criado", __func__, __LINE__);
        return cb;
    }
    RB_CATCH(ctx) {
        rb_log_push(LOG_ERR, rb_exc_message(ctx.exception_code), __func__, __LINE__);
        return 0;
    }
}

