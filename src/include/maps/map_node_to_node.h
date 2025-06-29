#ifndef __MAP_NODE_TO_NODE_H__
#define __MAP_NODE_TO_NODE_H__

#include <defs.h>

static inline void map_node_to_tcp_node(struct tcp_node *tcp, const struct node *src) {
    if (!tcp || !src)
        return;
    tcp->node = (struct node *)src;
    __vcpy(&(tcp->node_cfg.ipv4), &(src->ipv4), sizeof(struct sockaddr));
    __vcpy(&(tcp->node_cfg.ipv6), &(src->ipv6), sizeof(struct sockaddr_in6));
    tcp->node_cfg.port = src->port;
    tcp->node_cfg.family = src->family;
    tcp->node_cfg.fd = src->fd;
    tcp->node_cfg.arrival = src->arrival;
    tcp->node_cfg.deadline = src->deadline;
    tcp->node_cfg.value_size = src->value_size;
    tcp->node_cfg.flags = src->flags;
    __vcpy(tcp->node_cfg.value, src->value, sizeof(src->value));
}

static inline void map_node_to_udp_node(struct udp_node *udp, const struct node *src) {
    if (!udp || !src)
        return;
    udp->node = (struct node *)src;
    __vcpy(&(udp->node_cfg.ipv4), &(src->ipv4), sizeof(struct sockaddr));
    __vcpy(&(udp->node_cfg.ipv6), &(src->ipv6), sizeof(struct sockaddr_in6));
    udp->node_cfg.port = src->port;
    udp->node_cfg.family = src->family;
    udp->node_cfg.fd = src->fd;
    udp->node_cfg.arrival = src->arrival;
    udp->node_cfg.deadline = src->deadline;
    udp->node_cfg.value_size = src->value_size;
    udp->node_cfg.flags = src->flags;
    __vcpy(udp->node_cfg.value, src->value, sizeof(src->value));
}

static inline void map_node_to_node(struct node *dst, const struct node *src) {
    if (!dst || !src)
        return;
    dst->arrival = src->arrival;
    dst->deadline = src->deadline;
    __vcpy(&(dst->ipv4), &(src->ipv4), sizeof(struct sockaddr));
    __vcpy(&(dst->ipv6), &(src->ipv6), sizeof(struct sockaddr_in6));
    dst->fd = src->fd;
    dst->family = src->family;
    dst->port = src->port;
    dst->value_size = src->value_size;
    dst->flags = src->flags;
    __vcpy(dst->value, src->value, sizeof(src->value));
}

static inline void pop_struct_node(struct node *dst, struct node_field_arr *arr, unsigned count){
    if(dst && arr && count){
        register struct node_field_arr *a = arr, *a0 = a + count;
        do{
            switch (arr->type) {
                case node_arrival_t:
                    dst->arrival = arr->value.u64;
                    break;
                case node_deadline_t:
                    dst->deadline = arr->value.u64;
                    break;
                case node_value_size_t:
                    dst->value_size = arr->value.u32;
                    break;
                case node_family_t:
                    dst->family = (ezp_addr_type)arr->value.u32;
                    break;
                case node_fd_t:
                    dst->fd = (int)arr->value.u32;
                    break;
                case node_port_t:
                    dst->port = arr->value.u16;
                    break;
                case node_flags_t:
                    dst->flags = arr->value.u8;
                    break;
                case node_ipv6_t:
                    dst->ipv6 = arr->value.ipv6;
                    break;
                case node_ipv4_t:
                    dst->ipv4 = arr->value.ipv4;
                    break;
                case node_value_t:
                    __vcpy(dst->value, arr->value.value, sizeof(dst->value));
                    break;
                default:
                    break;
            }
        }while(++a < a0);
    }
}


#endif