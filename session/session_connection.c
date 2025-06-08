#include <session_connection.h>
#include <dmmr_circle_buffer.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static struct circle_buffer *circle_buffer = 0;

static struct session_connection_pool *pool = 0;

static uint16_t session_count = 0;

struct session_connection_pool_hash{
    uint16_t port;
    session_connection_pool *pool;
}session_connection_pool_hash[0x1000];

static inline void swap_nodes(struct node_circle_buffer *a, struct node_circle_buffer *b) {
    struct node temp = a->n;
    a->n = b->n;
    b->n = temp;
}

static void heapify_ptr(struct node_circle_buffer *base, size_t n, size_t i) {
    size_t smallest = i;
    size_t l = (i << 1) + 1; // left = 2*i + 1
    size_t r = (i << 1) + 2; // right = 2*i + 2

    struct node_circle_buffer *pi = base + i;
    struct node_circle_buffer *pl = base + l;
    struct node_circle_buffer *pr = base + r;
    struct node_circle_buffer *ps = base + smallest;

    if (l < n && pl->n.arrival < ps->n.arrival)
        smallest = l;

    ps = base + smallest;

    if (r < n && pr->n.arrival < ps->n.arrival)
        smallest = r;

    if (smallest != i) {
        swap_nodes(base + i, base + smallest);
        heapify_ptr(base, n, smallest);
    }
}

struct void sort_poll_by_arrival_ptr(struct node_circle_buffer *base, size_t total) {
    if (!base || total < 2)
        return;

    for (ssize_t i = (total >> 1) - 1; i >= 0; i--)
        heapify_ptr(base, total, (size_t)i);

    for (ssize_t i = total - 1; i > 0; i--) {
        swap_nodes(base, base + i);
        heapify_ptr(base, (size_t)i, 0);
    }
}


static void send_buffer(struct session_connection_pool *conn, size_t size) {
    if (!conn || !conn->poll || conn->pool_size == 0)
        return;

    unsigned rms = 0;
    sort_poll_by_arrival_ptr(conn->poll, conn->pool_size);

    struct node_circle_buffer *base = conn->poll;
    struct node_circle_buffer *p = base;
    struct node_circle_buffer *p0 = base + conn->pool_size;

    size_t total_bytes_sent = 0;
    size_t limit = size ? size : 9000;

    do {
        struct node *n = &p->n;
        if (total_bytes_sent + n->value_size > limit)
            break;

        printf("Enviando pacote: port=%u, fd=%d, size=%u, arrival=%llu, deadline=%llu\n",
               n->port, n->fd, n->value_size,
               (unsigned long long)n->arrival,
               (unsigned long long)n->deadline);

        total_bytes_sent += n->value_size;
        ++rms;
    } while (++p < p0);

    if (rms > 0 && rms <= conn->pool_size) {
        __bcpy(conn->poll, conn->poll + rms, (conn->pool_size - rms) * sizeof(struct node_circle_buffer));
        conn->pool_size -= rms;
    }
}




int start_session_connection(struct circle_buffer *__cb, uint16_t __ports){
	if(pool)
		return EOF;
	else{
		pool = (struct session_connection_pool*)calloc(__ports, sizeof(struct session_connection_pool));
		if(!pool)
			return EOF;
        circle_buffer = __cb;
        return 0;
	}
}

static void* session_worker(void* arg) {
    struct session_connection_pool* conn = (struct session_connection_pool*)arg;
    if (!conn || !conn->poll || !circle_buffer)
        return 0;
    struct node_circle_buffer *cursor = conn->cursor;
    struct circleq_head *head = &circle_buffer->head;
    do {
        pthread_mutex_lock(&conn->mutex);
        if(conn->cursor.port == conn->port)
            *(conn->poll + conn->pool_size++) = conn->cursor;
        conn->cursor =  circle_buffer->iterate(cursor, head);
        pthread_mutex_unlock(&conn->mutex);
        unsigned size = get_session_size(conn->port);
        if (size >= 9000 && conn->pool_size > 0) {
            pthread_mutex_lock(&conn->mutex);
            send_buffer(conn, 0);
            pthread_mutex_unlock(&conn->mutex);
        }
        usleep(0x01); // TODO Isso precisa ser escalonado de acordo com a formula de tempo que um curso percorre o circle_buffer
    }while(~0);
    return 0;
}


struct session_connection_pool *upsert_session(struct node_circle_buffer *cb, uint16_t port) {
    struct session_connection_pool_hash *entry = &session_connection_pool_hash[port & 0x0FFF];
    if(entry->pool) {
        pthread_mutex_lock(&entry->pool->mutex);
        *(entry->pool->poll + entry->pool->pool_size++) = *cb;
        pthread_mutex_unlock(&entry->poolp->mutex);
        return entry->pool;
    }
    else{
        entry->pool = pool + session_count++;
        entry->pool->port = port;
        entry->pool->pool_size = 0;
        entry->pool->poll = (struct node_circle_buffer*)calloc(0x400, sizeof(struct node_circle_buffer));
        if(!(entry->pool->poll))
            return 0;
        entry->pool->thread = 0;
        pthread_mutex_init(&entry->pool->mutex, NULL);
        entry->pool->cursor = cb;
        heapify_up(session_count - 1);
        session_connection_pool_hash[port & 0x0FFF].port = port;
        return entry->pool;
    }
}


void stop_session_connection(uint16_t port) {
    struct session_connection_pool_hash *entry = &session_connection_pool_hash[port & 0x0FFF];
    if (entry->port != port || !(entry->pool))
        return;

    struct session_connection_pool *p = entry->pool;

    // Cancelar e esperar a thread terminar, se ativa
    if (p->thread) {
        pthread_cancel(p->thread);
        pthread_join(p->thread, 0);
        p->thread = 0;
    }

    // Libera o poll se alocado
    if (p->poll) {
        free(p->poll);
        p->poll = 0;
    }

    // Destroi o mutex
    pthread_mutex_destroy(&p->mutex);

    // Marca como removido no hash
    entry->port = 0;
    entry->pool = 0;

    // Decrementa contador de sessões
    --session_count;
}


int reload_session_conection(uint16_t port){
    return 0;
}

struct node_circle_buffer *get_session(uint16_t port) {
    struct session_connection_pool_hash *entry = &session_connection_pool_hash[port & 0x0FFF];
    if (entry->port != port || !(entry->pool))
        return 0;
    return entry->pool->poll;
}

unsigned get_session_size(uint16_t port) {
    struct session_connection_pool_hash *entry = &session_connection_pool_hash[port & 0x0FFF];

    if (entry->port != port || !(entry->pool))
        return 0;

    struct session_connection_pool *p = entry->pool, *p0 = p + entry->pool->pool_size;
    unsigned total = 0;

    do {
        total += p->n.value_size;
    } while(++p < p0);

    return total;
}

void session_connection_trigger_send(struct session_connection *conn) {
    if (!conn)
        return;
    else
        send_buffer(conn, get_session_size(conn->session_ptr->port));
}

