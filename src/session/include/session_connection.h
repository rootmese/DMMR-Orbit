#ifndef __SESSION_CONNECTION_H__
#define __SESSION_CONNECTION_H__

#include <defs.h>
#include <dmmr_circle_buffer.h>

int start_session_connection(struct dmmr_circle_buffer*);

void stop_session_connection(void);

int reload_session_conection(void); //não será contemplado no MVC

struct session_connection_pool *get_session(union protocol_base_cb*);

int insert_session(struct session_connection_pool*);

void delete_session(struct session_connection_pool*, int);

void session_connection_trigger_send(void *ptr);

struct session_connection_pool *get_recno_slot(void);

struct session_connection_pool *get_recno_slot(void);

void set_snd_cb(void (*send_cb)(union protocol_base_cb *session, struct node*, unsigned size));

#endif