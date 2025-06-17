#ifndef __DNS_UTILS_H__
#define __DNS_UTILS_H__

#include <__vcpy.h>

#include <defs.h>

static inline ezp_addr_type dns2ipaddr(const char *in, unsigned char *out) {
    int status;
    char ipstr[INET6_ADDRSTRLEN];
    struct addrinfo hints, *res, *p;

    hints.ai_family = AF_UNSPEC; // IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(in, 0, &hints, &res);
    if(!(status == 0))
        return EZP_INVALID;
    for(p = res; p; p = p->ai_next) {
        switch(p->ai_family){
            case AF_INET:
                __vcpy(out, (struct sockaddr_in*)p->ai_addr, sizeof(struct sockaddr_in));
                freeaddrinfo(res);
                return (ezp_addr_type)AF_INET;
                break; /* Stupid break :P ~*/
            case AF_INET6:
                __vcpy(out, (struct sockaddr_in6*)p->ai_addr, sizeof(struct sockaddr_in6));
                freeaddrinfo(res);
                return (ezp_addr_type)AF_INET6;
                break; /* Stupid break :P ~*/
            default:
                continue;
        }
    }
    freeaddrinfo(res);
    return EZP_INVALID;
}

#endif