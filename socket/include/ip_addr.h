#ifndef __IP_ADDR_H__
#define __IP_ADDR_H__

#include <defs.h>

static inline ezp_addr_type ezp_address(const char *input) {
    struct in_addr ipv4;
    struct in6_addr ipv6;
    if (inet_pton(AF_INET, input, &ipv4) == 1) {
        return EZP_IPV4;
    }
    if (inet_pton(AF_INET6, input, &ipv6) == 1) {
        return EZP_IPV6;
    }
    for (size_t i = 0; i < strlen(input); i++) {
        if (!isalnum(input[i]) && input[i] != '-' && input[i] != '.') {
            return EZP_INVALID;
        }
    }
    return EZP_DNS;
}

#endif
