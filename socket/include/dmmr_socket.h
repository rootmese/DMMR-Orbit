#ifndef __DMMR_SOCKET_H__
#define __DMMR_SOCKET_H__

#include <dmmr_circle_buffer.h>
#include <dmmr_scheduler.h>  //TODO as rotinas que usam scheduler devem estar as plugins, método temporário

struct dmmr_socket{
	void (*dispatcher)(union protocol_base_cb*, struct node*, unsigned);
	int (*create_dispatcher_from_uri)(const unsigned char*);
	int (*start_acception)(proto_t, uint16_t, const char*);
	int (*start_accept_from_uri)(const unsigned char*);
	void (*set_acception_cb_tcp)(void (*on_accept_cb)(struct tcp_node*));
	void (*set_acception_cb_udp)(void (*on_accept_cb)(struct udp_node*));
	void (*scheduler)(struct dmmr_scheduler*); //TODO as rotinas que usam scheduler devem estar as plugins, método temporário
	void (*reload)(struct cfg_server_server*);
};

struct dmmr_socket *new_dmmr_socket(struct circle_buffer*, struct cfg_server_server*);

void delete_dmmr_socket(void);


#endif
