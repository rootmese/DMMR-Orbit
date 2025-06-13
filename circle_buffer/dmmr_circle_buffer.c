#include <dmmr_circle_buffer.h>

static struct node_circle_buffer* buffer = 0;

static void* fifo_worker(void* arg) {
    struct circle_buffer* this = (struct circle_buffer*)arg;
    struct node_circle_buffer* n = 0;

    do {
        while (!TAILQ_EMPTY(this->fifo)) {
            n = get_current_node(this);
            pthread_mutex_lock(&(this->fifo_lock));
            struct node_buffer* f = TAILQ_FIRST(this->fifo);
            if (n && f)
            {
                __vcpy(n, f->n, sizeof(stuct node));
                TAILQ_REMOVE(&(this->fifo), f, tailq);
            }
            pthread_mutex_unlock(&(this->fifo_lock));
        }
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        if (timespec_less(&now, &next_sleep)) {
            struct timespec remaining = {
                .tv_sec = next_sleep.tv_sec - now.tv_sec,
                .tv_nsec = next_sleep.tv_nsec - now.tv_nsec
            };
            if (remaining.tv_nsec < 0) {
                remaining.tv_sec--;
                remaining.tv_nsec += 1000000000;
            }
            if (remaining.tv_sec > 0 || remaining.tv_nsec > 18) {
                nanosleep(&remaining, NULL);
            }
        } else {
            next_sleep.tv_sec = 0;
            next_sleep.tv_nsec = 0;
        }
    } while (!0);
    return 0;
}

void enqueue(struct circle_buffer* this, struct node_buffer* n) {
    if (n){
        pthread_mutex_lock(&(this->fifo_lock));
        TAILQ_INSERT_TAIL(&(this->fifo), n, tailq);
        pthread_mutex_unlock(&this->fifo_lock);
    }
}


struct node_circle_buffer *iterate(struct node_circle_buffer *cursor, struct circleq_head *head) {
    struct node_circle_buffer *next = CIRCLEQ_NEXT(cursor, circleq);
    return (next == (struct node_circle_buffer *)CIRCLEQ_END(head)) ? CIRCLEQ_FIRST(head) : next;
}

/*
// Inicialização (cursor DEVE apontar para elemento válido)
struct node_circle_buffer *cursor = CIRCLEQ_FIRST(&buffer->head);

do {
    // Processa o nó atual
    process_node(cursor);
    
    // Avança para próximo elemento (reinicia automaticamente)
    cursor = circle_iterate(cursor, &buffer->head);

} while (cursor != stop_condition);  // Sua condição de parada
*/



static struct node_circle_buffer* get_current_node(struct circle_buffer* this) {
    if(this){
        struct node_circle_buffer* current = this->cursor;
        this->cursor = CIRCLEQ_NEXT(current, circleq);
        if (this->cursor == CIRCLEQ_END(&(this->head))
            this->cursor = CIRCLEQ_FIRST(&(this->head));
        return current;
    }
    else
        return 0;
}

int start(struct circle_buffer *this){
    if(this){
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);  // Escalonador em tempo real
        param.sched_priority = 80;  // Prioridade entre 1 e 255 (quanto maior, mais prioridade)
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);  // Aplica a prioridade explicitamente
        pthread_mutex_init(&this->fifo_lock, 0);
        pthread_create(&(this->fifo_thread), 0, fifo_worker, this);
        return 0;
    }
    else{
        return EOF;
    }
}

void stop(struct circle_buffer *this){}

struct circle_buffer* new_circle_buffer(size_t size) {
    struct circle_buffer* cb = calloc(1, sizeof(struct circle_buffer));
    if (!cb)
        return 0;

    cb->buffer_size = size;
    buffer = calloc(size, sizeof(struct node_circle_buffer));
    if (!buffer) {
        free(cb);
        return 0;
    }
    CIRCLEQ_INIT(&(cb->head));
    TAILQ_INIT(&(cb->fifo)); 
    struct node_circle_buffer *prev = 0;
    node_circle_buffer *p = buffer, *p1 = p + cb->buffer_size;
    do
    {
        CIRCLEQ_INSERT_TAIL(&(cb->head), p, circleq);
        if (prev)
            item->prev_ptr = prev;
        prev = item;
    } while (++p < p1);

    cb->cursor = CIRCLEQ_EMPTY(&cb->head) ? 0 : CIRCLEQ_FIRST(&cb->head);
    CIRCLEQ_FIRST(&cb->head)->prev_ptr = CIRCLEQ_LAST(&cb->head);

    cb->enqueue = enqueue;
    cb->get_current_node = get_current_node;
    cb->iterate = iterate;
    cb->start = start;
    cb->stop = stop;
    return cb;
}

