#ifndef __ENUMS_H__
#define __ENUMS_H__

#include <defs.h>

typedef enum{
    proto_none_t   = 0x00,
    proto_udp_t    = 0x01,
    proto_tcp_t    = 0x02
}proto_t;

typedef enum{
    proto_arr_udp_t    = 0x00,
    proto_arr_tcp_t    = 0x01
}proto_arr_t;

typedef enum {
    EZP_DNS     = 0,
    EZP_IPV4    = AF_INET,
    EZP_IPV6    = AF_INET6,
    EZP_INVALID = 0xFFFF
} ezp_addr_type;

typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_IPADDR,
    TOKEN_EQUALS
} dmmt_token_type;

typedef enum {
    rb_log_level_fatal = 0x01,
    rb_log_level_error = 0x02,
    rb_log_level_warn  = 0x04,
    rb_log_level_info  = 0x08,
    rb_log_level_debug = 0x10
} rb_log_level;

#endif