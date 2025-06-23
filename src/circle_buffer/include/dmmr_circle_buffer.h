#ifndef __CIRCLE_BUFFER_H__
#define __CIRCLE_BUFFER_H__

#include <defs.h>

TAILQ_HEAD(tailq_fifo, node_fifo_buffer);
CIRCLEQ_HEAD(circleq_head, node_circle_buffer);

struct node_fifo_buffer {
    struct node *n;
    TAILQ_ENTRY(node_fifo_buffer) tailq;
};

struct node_circle_buffer {
    struct node n;
    CIRCLEQ_ENTRY(node_circle_buffer) circleq;
    struct node_circle_buffer* prev_ptr;
};



struct dmmr_circle_buffer {
    struct node_circle_buffer* cursor;
    struct circleq_head head;
    struct tailq_fifo fifo;
    pthread_mutex_t fifo_lock;
     pthread_cond_t fifo_cond;
    pthread_t fifo_thread;
    int (*start)(void);
    void (*stop)(void);
    int (*is_behind_cursor)(struct node_circle_buffer*);
    struct node_circle_buffer* (*get_current_node)(void);
    struct node_circle_buffer* (*iterate)(struct node_circle_buffer *cursor, struct circleq_head *head);
};

struct dmmr_circle_buffer* new_circle_buffer(struct cfg_server_server*);

#endif