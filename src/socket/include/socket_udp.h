#ifndef __SOCKET_UDP_H__
#define __SOCKET_UDP_H__

struct udp_node *get_udp_node(void);

int udp_send_to_client(struct node*, size_t);

int start_udp_service(struct udp_node*);

int connect_udp_server(void);

void disconnect_udp_server(struct udp_node*);

int start_udp_socket(void);

void stop_udp_socket(void);

int udp_server_is_active(void);

#endif
