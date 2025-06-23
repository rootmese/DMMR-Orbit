#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <defs.h>

struct tcp_node *get_tcp_node(void);

int tcp_send_to_client(struct tcp_node*, struct node*,  size_t);

int start_tcp_service(struct tcp_node*);

int connect_tcp_server(struct tcp_node*);

void disconnect_tcp_server(struct tcp_node*);

int start_tcp_socket(void);

void stop_tcp_socket(void);

#endif
