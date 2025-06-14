#ifndef __PARSE_URI_H__
#define __PARSE_URI_H__

#include <defs.h>

// Defina HAVE_STRLCPY se seu sistema já tiver a função
#ifndef HAVE_STRLCPY
// #define HAVE_STRLCPY 1
#endif

#include <strlcpy.h>

#include <string.h>

static proto_t parse_protocol_host_port(const char *input, char *host_out, size_t host_size, uint16_t *port_out) {
    if (!input || !host_out || host_size == 0 || !port_out)
        return proto_none_t;
    proto_t proto = proto_none_t;
    if (strncmp(input, "UDP/", 4) == 0) {
        proto = proto_udp_t;
        input += 4;
    } else if (strncmp(input, "TCP/", 4) == 0) {
        proto = proto_tcp_t;
        input += 4;
    } else {
        return proto_none_t;
    }
    const char *sep = strrchr(input, ':');
    if (!sep || sep == input)
        return proto_none_t;
    size_t host_len = sep - input;
    char tmp_host[host_len + 1];
    strncpy(tmp_host, input, host_len);
    strlcpy(host_out, tmp_host, host_size);
    int port_num = atoi(sep + 1);
    if (port_num <= 0 || port_num > 65535)
        return proto_none_t;
    *port_out = (uint16_t)port_num;
    return proto;
}


#endif
