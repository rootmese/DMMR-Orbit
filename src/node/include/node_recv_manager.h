#ifndef __NODE_RECV_MANAGER_H__
#define __NODE_RECV_MANAGER_H__

#include <pthread.h>

#include <defs.h>

int init_node_recv_manager(struct node_recv_manager*);

void stop_node_recv_manager(struct node_recv_manager*);

struct node *get_free_node(struct node_recv_manager*);

struct node *get_buzy_node(struct node_recv_manager*);

struct node *copy_buffer(struct node_recv_manager*, struct node*, unsigned*);

#endif
