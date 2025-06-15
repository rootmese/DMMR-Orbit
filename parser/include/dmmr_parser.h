#ifndef __DMMR_PARSER_H__
#define __DMMR_PARSER_H__

#include <dmmr_parser.h>

typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_IPADDR,
    TOKEN_EQUALS
} dmmt_token_type;

struct dmmt_token {
    dmmt_token_type type;
    char value[0x100];
};

struct dmmr_parser {
    int (*load)(void);
    void (*run)(void);
    void (*stop)(void);
    void (*handle_token)(struct dmmr_parser*, const char*, const char*);
};

struct dmmr_parser* new_dmmr_parser(struct cfg_daemon_server*);

#endif 
