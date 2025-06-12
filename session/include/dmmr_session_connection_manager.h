#ifndef __DMMR_SESSION_CONNECTION_MANAGER_H__
#define __DMMR_SESSION_CONNECTION_MANAGER_H__

#include <dmmr_scheduler.h>
#include <dmmr_circle_buffer.h>

struct dmmr_session_connection_manager{
	void (*reload)(struct cfg_server_server*);
	void (*enqueue)(struct circle_buffer *cb, struct node_circle_buffer* n); //callback to circle_buffer insert data
	void (*trigger_send)(struct dmmr_scheduler*, struct cfg_server_server*);
};

struct dmmr_session_connection_manager* new_session_connection_manager(struct circle_buffer*, struct dmmr_scheduler*, struct dmmr_socket*, struct cfg_server_server*);

#endif