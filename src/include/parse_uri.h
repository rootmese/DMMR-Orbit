#ifndef __PARSE_URI_H__
#define __PARSE_URI_H__

// Defina HAVE_STRLCPY se seu sistema já tiver a função
#ifndef HAVE_STRLCPY
// #define HAVE_STRLCPY 1
#endif

#include <strlcpy.h>
#include <string.h>
#include "defs.h"

#define __UDP_PROTO__         "UDP/"
#define __UDP_PROTO_LEN__    (sizeof(__UDP_PROTO__) - 1)

#define __TCP_PROTO__         "TCP/"
#define __TCP_PROTO_LEN__    (sizeof(__TCP_PROTO__) - 1)

#define __HOST_URI_SEP__     ':'

static inline proto_t parse_protocol_host_port(const unsigned char *input, char unsigned *host_out, size_t host_size, uint16_t *port_out) {
    if (!input || !host_out || host_size == 0 || !port_out)
        return proto_none_t;
    proto_t proto = proto_none_t;
    if (strncmp((char*)input, __UDP_PROTO__, __UDP_PROTO_LEN__) == 0) {
        proto = proto_udp_t;
        input += __UDP_PROTO_LEN__;
    }
    else if (strncmp((char*)input, __TCP_PROTO__, __TCP_PROTO_LEN__) == 0) {
        proto = proto_tcp_t;
        input += __TCP_PROTO_LEN__;
    }
    else
        return proto_none_t;
    const char *sep = strrchr(input, __HOST_URI_SEP__);
    if (!sep || sep == input)
        return proto_none_t;
    size_t host_len = (uint8_t*)sep - (uint8_t*)input;
    char *tmp_host = input + host_len + 1;
    strlcpy(tmp_host, input, host_len);
    strlcpy(host_out, tmp_host, host_size);
    *port_out = (uint16_t)atoi(sep + 1);
    return proto;
}


#endif
