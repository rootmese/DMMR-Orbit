#ifndef __SESSION_CONNECTION_H__
#define __SESSION_CONNECTION_H__

#include <dmmr_circle_buffer.h>

struct session_connection_pool {
    uint16_t port;
    unsigned pool_size;
    struct node_circle_buffer *poll;
    struct node_circle_buffer *cursor;
    pthread_t *thread;
    pthread_mutex_t mutex;
};

int start_session_connection(struct circle_buffer*, uint16_t);// o segundo parâmetro é o número de portas que o sistemas estará escutando para já fazer o buffer

void stop_session_conection(void);

int reload_session_conection(uint16_t); //não será contemplado no MVC

struct session_connection_pool *upsert_session(struct node_circle_buffer*, uint16_t);

void delete_session(uint16_t);

struct node_circle_buffer *get_session(uint16_t);

unsigned get_session_size(uint16_t);

void session_connection_trigger_send(void *ptr);

// TODO necessário criar tudo, como funcionará:
// O Session Manager chama o upsert_session a sessão é criada ou inserida e fica consultando o método iterate do circle_buffer;
// o dmmr_session_connection_manager faz o proxy entre o session_connection e o restante do sistema.
// cada sessão é aberta em um thread


#endif