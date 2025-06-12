#ifndef ____NODE_CMP_H__
#define ____NODE_CMP_H__

#include <string.h>
#include <arpa/inet.h>
#ifndef ____NODE_CMP_H__
#define ____NODE_CMP_H__

#include <arpa/inet.h>
#include <vcmp.h>  // certifique-se de incluir corretamente

static inline int node_cmp(const struct node *a, const struct node *b) {
    if (!a || !b) return (a > b) - (a < b);

    if (a->fd != b->fd)
        return (a->fd > b->fd) - (a->fd < b->fd);

    if (a->port != b->port)
        return (a->port > b->port) - (a->port < b->port);

    if (a->family != b->family)
        return (a->family > b->family) - (a->family < b->family);

    if (a->family == AF_INET) {
        const struct sockaddr_in *sa = (const struct sockaddr_in *)&a->ipv4;
        const struct sockaddr_in *sb = (const struct sockaddr_in *)&b->ipv4;
        return __vcmp(&sa->sin_addr, &sb->sin_addr, sizeof(struct in_addr));
    }

    if (a->family == AF_INET6) {
        const struct sockaddr_in6 *sa6 = (const struct sockaddr_in6 *)&a->ipv6;
        const struct sockaddr_in6 *sb6 = (const struct sockaddr_in6 *)&b->ipv6;
        return __vcmp(&sa6->sin6_addr, &sb6->sin6_addr, sizeof(struct in6_addr));
    }

    return 0;
}
#endif



#endif