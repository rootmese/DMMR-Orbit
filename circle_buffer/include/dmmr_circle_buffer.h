#ifndef __CIRCLE_BUFFER_H__
#define __CIRCLE_BUFFER_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>

#include <defs.h>

struct node_buffer {
    struct node n;
    TAILQ_ENTRY(node_buffer) tailq;
};

struct node_circle_buffer {
    struct node n;
    CIRCLEQ_ENTRY(node_circle_buffer) circleq;
};



struct circle_buffer {
    
    struct node_circle_buffer* cursor;
    struct circleq_head head;
    struct tailq_fifo fifo;

    size_t buffer_size;
    size_t count;
    pthread_mutex_t fifo_lock;
    pthread_t fifo_thread;

    void (*enqueue)(struct circle_buffer*, struct node_buffer*);
    struct node* (*get_current_node)(struct circle_buffer*);
    struct node_circle_buffer* (*iterate)(struct node_circle_buffer *cursor, struct circleq_head *head);
};

struct circle_buffer* new_circle_buffer(size_t);

#endif