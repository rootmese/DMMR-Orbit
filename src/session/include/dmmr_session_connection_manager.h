#ifndef __DMMR_SESSION_CONNECTION_MANAGER_H__
#define __DMMR_SESSION_CONNECTION_MANAGER_H__

#include <funcs.h>
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
	int (*socket_start_accept_from_uri)(
		const unsigned char*,
    void (*on_accept_udp_cb)(struct udp_node*),
    void (*on_accept_tcp_cb)(struct tcp_node*),
    void (*on_connect_udp_cb)(struct udp_node*),
    void (*on_connect_tcp_cb)(struct tcp_node*),
	void (*on_close_udp_cb)(struct udp_node*),
	void (*on_close_tcp_cb)(struct tcp_node*)
		);
	int (*socket_create_dispatcher_from_uri)(
		const unsigned char*,
    void (*on_accept_udp_cb)(struct udp_node*),
    void (*on_accept_tcp_cb)(struct tcp_node*),
    void (*on_connect_udp_cb)(struct udp_node*),
    void (*on_connect_tcp_cb)(struct tcp_node*),
	void (*on_close_udp_cb)(struct udp_node*),
	void (*on_close_tcp_cb)(struct tcp_node*)
		);
	void (*trunk)(const unsigned char*, const unsigned char*);
	void (*sm_delete_session)(union protocol_base_cb*);
	struct session_connection_pool* (*get_session)(union protocol_base_cb*);
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