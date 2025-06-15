#ifndef __SESSION_CONNECTION_H__
#define __SESSION_CONNECTION_H__

#include <defs.h>

struct session_connection_pool {
    uint8_t run;
    uint16_t port;
    uint32_t pool_size;
    uint32_t pool_count;
    struct node *pool;
    struct node_circle_buffer *cursor;
    union protocol_base_cb session;
    pthread_t thread;
    pthread_mutex_t mutex;
};

int start_session_connection(struct circle_buffer*);

void stop_session_connection(struct session_connection_pool*, int);

int reload_session_conection(uint16_t); //não será contemplado no MVC

int insert_session(struct session_connection_pool*);

void delete_session(struct session_connection_pool*);

void session_connection_trigger_send(void *ptr);

struct session_connection_pool *get_recno_slot(void);

#endif