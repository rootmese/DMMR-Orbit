#ifndef __PROTOCOL_NODE_MAPPER_H__
#define __PROTOCOL_NODE_MAPPER_H__

#include <defs.h>

static inline void map_common_fields(struct node *src, struct node *dst) {
    if (!src || !dst)
       return;
    dst->arrival     = src->arrival;
    dst->deadline    = src->deadline;
    dst->fd          = src->fd;
    dst->family      = src->family;
    dst->port        = src->port;
    dst->value_size  = src->value_size;
    dst->flags       = src->flags;
    __vcpy(&dst->ipv4, &src->ipv4, sizeof(struct sockaddr));
    __vcpy(&dst->ipv6, &src->ipv6, sizeof(struct sockaddr_in6));
    __vcpy(dst->value, src->value, sizeof(src->value));
}

static inline void map_to_tcp_node(struct node *src, struct tcp_node *dst_tcp) {
    if (!src || !dst_tcp)
        return;
    map_common_fields(src, &dst_tcp->node_cfg);
    dst_tcp->node = src;
}

static inline void map_to_udp_node(struct node *src, struct udp_node *dst_udp) {
    if (!src || !dst_udp)
        return;
    map_common_fields(src, &dst_udp->node_cfg);
    dst_udp->node = src;
}

static inline void map_to_protocol_node(struct node *src, union protocol_base_cb *dst) {
    if (!src || !dst) return;
    switch (dst->none.proto) {
        case proto_tcp_t:
            map_to_tcp_node(src, &dst->tcp);
            break;
        case proto_udp_t:
            map_to_udp_node(src, &dst->udp);
            break;
        default:
            break;
    }
}

#endif 