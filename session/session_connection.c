#include <session_connection.h>
#include <dmmr_circle_buffer.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static struct circle_buffer *circle_buffer = 0;

static unsigned session_connection_pool_recno_count = 0;

static unsigned session_connection_pool_recno_size = 0;

static void (*snd_cb)(union protocol_base_cb *session, struct node*, unsigned size) = 0;

struct session_connection_pool_recno{
    uint16_t port;
    struct session_connection_pool *pool;
};

static struct session_connection_pool_recno *recno = 0;

struct session_connection_pool *get_recno_slot(void) {
    register struct session_connection_pool_recno *p = recno;
    register struct session_connection_pool_recno *p1 = p + session_connection_pool_recno_size;
    for (; p < p1; ++p)
        if (!(p->run))
            return p;
    if (session_connection_pool_recno_count >= udp_pool_size) {
        session_connection_pool_recno_size * 2;
        udp_pool = (struct udp_node*)realloc(udp_pool, udp_pool_size * sizeof(struct udp_node));
        if (!udp_pool)
            return 0;
    }
    return udp_pool + udp_pool_count++;
}

// Changed parameter types from node_circle_buffer to node
static inline void swap_nodes(struct node *a, struct node *b) {
    struct node temp = *a;
    *a = *b;
    *b = temp;
}

// Changed base type from node_circle_buffer to node
static void heapify_ptr(struct node *base, size_t n, size_t i) {
    size_t smallest = i;
    size_t l = (i << 1) + 1; // left = 2*i + 1
    size_t r = (i << 1) + 2; // right = 2*i + 2

    struct node *pi = base + i;
    struct node *pl = base + l;
    struct node *pr = base + r;
    struct node *ps = base + smallest;

    if (l < n && pl->arrival < ps->arrival)
        smallest = l;

    ps = base + smallest;

    if (r < n && pr->arrival < ps->arrival)
        smallest = r;

    if (smallest != i) {
        swap_nodes(base + i, base + smallest);
        heapify_ptr(base, n, smallest);
    }
}

// Changed base type from node_circle_buffer to node
void sort_poll_by_arrival_ptr(struct node *base, size_t total) {
    if (!base || total < 2)
        return;

    for (ssize_t i = (total >> 1) - 1; i >= 0; i--)
        heapify_ptr(base, total, (size_t)i);

    for (ssize_t i = total - 1; i > 0; i--) {
        swap_nodes(base, base + i);
        heapify_ptr(base, (size_t)i, 0);
    }
}

// Changed base type from node_circle_buffer to node
static void heapify_up(struct node *base, size_t index) {
    while (index > 0) {
        size_t parent_index = (index - 1) / 2;
        struct node *node = base + index;
        struct node *parent = base + parent_index;

        if (parent->arrival <= node->arrival)
            break;

        swap_nodes(parent, node);
        index = parent_index;
    }
}

// Changed parameter types from node_circle_buffer to node
static void heap_insert(struct node *base, size_t *size, struct node *value) {
    // Coloca o novo valor na posição final do heap
    struct node *dest = base + *size;
    *dest = *value;

    // Ajusta para manter a propriedade de heap
    heapify_up(base, *size);

    (*size)++;
}

// Changed return type from node_circle_buffer to node
static struct node heap_pop_min(struct node *base, size_t *size) {
    if (*size == 0)
        return (struct node){0};  // ou algum valor padrão/nulo

    struct node min = *base;  // raiz

    // Move o último para a raiz
    struct node *last = base + (*size - 1);
    *base = *last;

    (*size)--;

    // Refaz heap a partir da raiz
    size_t i = 0;
    size_t n = *size;

    while (1) {
        size_t left = (i << 1) + 1;
        size_t right = (i << 1) + 2;
        size_t smallest = i;

        struct node *current = base + i;

        if (left < n && (base + left)->arrival < current->arrival)
            smallest = left;

        if (right < n && (base + right)->arrival < (base + smallest)->arrival)
            smallest = right;

        if (smallest == i)
            break;

        swap_nodes(base + i, base + smallest);
        i = smallest;
    }

    return min;
}

// Changed poll handling to use node instead of node_circle_buffer
static void* session_worker(void* arg) {
    struct session_connection_pool* conn = (struct session_connection_pool*)arg;
    if (!conn || !conn->poll || !circle_buffer)
        return 0;
    struct node_circle_buffer *cursor = conn->cursor;
    struct circleq_head *head = &circle_buffer->head;
    do {
        if(conn->pool_count + 1 > conn->pool_size){
            pthread_mutex_lock(&conn->mutex); // pouco entrará aqui, garantia
            conn->pool_size *= 2;
            conn->poll = (struct node*)realloc(conn->poll,conn->pool_size * sizeof(struct node));
            if(!(conn->poll)){
                pthread_mutex_unlock(&conn->mutex);
                goto return_fail;
            }
            pthread_mutex_unlock(&conn->mutex);
        }
        if(!(node_cmp(conn->cursor, get_session_node(conn->session)))){
            (conn->poll + conn->pool_count) = conn->cursor->n;
            conn->pool_count++;
            unsigned size = get_session_size(conn);
            if (size >= 9000 && conn->pool_size > 0) {
                unsigned count = 0;
                unsigned siz = 0;
                struct node *n = conn->poll, *n0 = n + conn->pool_count;
                do{
                    siz += n->value_size;
                    ++count;
                }while(siz <= 9000 && ++n < n0);
                if(snd_cb)
                    snd_cb(conn->session, conn->pool, count);
                if(count <= conn->pool_count){
                    pthread_mutex_lock(&conn->mutex); // funções inline, melhor deixar lock
                    __bcpy(conn->poll + count, conn->poll, conn->pool_count - count * sizeof(struct node*));
                    conn->pool_count -= count;
                    update_session_counter(conn->session);
                    sort_poll_by_arrival_ptr(conn->poll, conn->count);
                    pthread_mutex_unlock(&conn->mutex);
                }
                else
                    conn->pool_count = 0;
                conn->cursor = circle_buffer->iterate(cursor, head);
            }
        }
        usleep(0x12); // TODO Isso precisa ser escalonado de acordo com a formula de tempo que um curso percorre o circle_buffer
    }while(conn->run);
    return_fail:
    return 0;
}

int insert_session(struct node_circle_buffer *cb, struct session_connection_pool *session) {
    if(!cb || !session)
        return EOF;
    struct session_connection_pool *entry = session;
    // Allocate array of nodes instead of node_circle_buffer
    entry->pool->poll = (struct node*)calloc(0x400, sizeof(struct node));
    if(!(entry->pool->poll))
        return EOF;
    entry->pool->pool_size = 0x400;
    entry->pool->pool_count = 0;
    entry->pool->run = !0;
    entry->pool->cursor = CIRCLEQ_FIRST(&(cb->head));
    (void)pthread_create(&entry->pool->thread, 0, session_worker, entry->pool);
        return 0;
}

int reload_session_conection(uint16_t port){
    return 0;
}

struct node *get_session(uint16_t port) {
    struct session_connection_pool_recno *entry = &recno[port & 0x0FFF];
    if (entry->port != port || !(entry->pool))
        return 0;
    return entry->pool->poll;  // Now returns node array
}

unsigned get_session_size(struct session_connection_pool *p) {
    if(p){
        struct node *p0->pool = p, *p1 = p0 + p->pool->pool_count;
        unsigned total = 0;
        do {
            total += p->value_size;
        } while(++p < p0);
        return total;
    }
    else
        return 0;
}

void set_snd_cb(void (*send_cb)(union protocol_base_cb *session, struct node*, unsigned size)){
    snd_cb = send_cb;
}

void stop_session_connection(struct session_connection_pool *p) {
    if(p){
        p->run = 0;
        sleep(1);
        if(p->poll)
            free(p->poll);
        memset(p, 0, sizeof(struct session_connection_pool_recno));
    }
}

int start_session_connection(struct circle_buffer *__cb){
    if(recno)
        return EOF;
    else{
        recno = (struct session_connection_pool_recno*)calloc(0x400, sizeof(struct session_connection_pool_recno));
        if(!recno)
            return EOF;
        session_connection_pool_recno_size = 0x400;
        circle_buffer = __cb;
        return 0;
    }
}