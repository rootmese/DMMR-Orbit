#ifndef __SOCKET_UDP_H__
#define __SOCKET_UDP_H__

struct udp_node *get_udp_node(void);

int start_udp_service(struct tcp_node*);

int connect_udp_server(struct tcp_node*, const char*);

void disconnect_udp_server(struct tcp_node*);

int start_udp_socket(void);

void stop_udp_socket(void);

#endif
