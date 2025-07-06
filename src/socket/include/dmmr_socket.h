#ifndef __DMMR_SOCKET_H__
#define __DMMR_SOCKET_H__

#include <funcs.h>
#include <dmmr_circle_buffer.h>

struct dmmr_socket{
	void (*dispatcher)(union protocol_base_cb*, struct node*, unsigned);
	int (*create_dispatcher_from_uri)(
		const unsigned char*,
    void (*on_accept_udp_cb)(struct udp_node*),
    void (*on_accept_tcp_cb)(struct tcp_node*),
    void (*on_connect_udp_cb)(struct udp_node*),
    void (*on_connect_tcp_cb)(struct tcp_node*),
	void (*on_close_udp_cb)(struct udp_node*),
	void (*on_close_tcp_cb)(struct tcp_node*)
		);
	int (*start_acception)(
		proto_t,
		uint16_t,
		const char*,
    void (*on_accept_udp_cb)(struct udp_node*),
    void (*on_accept_tcp_cb)(struct tcp_node*),
    void (*on_connect_udp_cb)(struct udp_node*),
    void (*on_connect_tcp_cb)(struct tcp_node*),
	void (*on_close_udp_cb)(struct udp_node*),
	void (*on_close_tcp_cb)(struct tcp_node*)
		);
	int (*start_accept_from_uri)(
		const unsigned char*,
    void (*on_accept_udp_cb)(struct udp_node*),
    void (*on_accept_tcp_cb)(struct tcp_node*),
    void (*on_connect_udp_cb)(struct udp_node*),
    void (*on_connect_tcp_cb)(struct tcp_node*),
	void (*on_close_udp_cb)(struct udp_node*),
	void (*on_close_tcp_cb)(struct tcp_node*)
		);
	void (*reload)(void);
};

struct dmmr_socket *new_dmmr_socket(struct dmmr_circle_buffer*, struct cfg_server_server*);

void delete_dmmr_socket(void);


#endif
