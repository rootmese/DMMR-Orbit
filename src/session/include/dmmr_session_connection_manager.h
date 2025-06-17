#ifndef __DMMR_SESSION_CONNECTION_MANAGER_H__
#define __DMMR_SESSION_CONNECTION_MANAGER_H__

#include <defs.h>
#include <dmmr_circle_buffer.h>
#include <dmmr_scheduler.h>
#include <dmmr_socket.h>
#include <session_connection.h>

struct dmmr_session_connection_manager{
	void (*reload)(struct cfg_server_server*);
	int (*connect)(const unsigned char*);
	int (*accept)(const unsigned char*);
	void (*close)(const unsigned char*);
	int (*send)(const unsigned char*, unsigned);
	int (*receive)(const unsigned char*, unsigned*);
	int (*socket_start_accept_from_uri)(const unsigned char*);
	int (*socket_create_dispatcher_from_uri)(const unsigned char*);
	void (*trunk)(const unsigned char*, const unsigned char*);
	void (*set_socket_acception_cb_tcp)(void (*on_accept_cb)(struct tcp_node*));
	void (*set_socket_acception_cb_udp)(void (*on_accept_cb)(struct udp_node*));
	void (*set_socket_dispatch_cb_tcp)(void (*on_accept_cb)(struct tcp_node*));
	void (*set_socket_dispatch_cb_udp)(void (*on_accept_cb)(struct udp_node*));
	int (*insert_session)(struct session_connection_pool*);
	void (*delete_session)(struct session_connection_pool*);
	int (*insert_scheduler)(struct session_connection_pool*);
	void (*delete_scheduler)(struct session_connection_pool*);
};

struct dmmr_session_connection_manager* new_session_connection_manager
(
	struct dmmr_circle_buffer*,
	struct dmmr_scheduler*,
	struct dmmr_socket*,
	struct cfg_server_server*
);

#endif