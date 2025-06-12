#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <defs.h>

struct tcp_node *get_tcp_node(void);

ssize_t tcp_send_to_client(struct node*,  size_t);

int start_tcp_service(struct tcp_node*);

int connect_tcp_server(struct tcp_node*, const char*);

void disconnect_tcp_server(struct tcp_node*);

int start_tcp_socket(void);

void stop_tcp_socket(void);

#endif
