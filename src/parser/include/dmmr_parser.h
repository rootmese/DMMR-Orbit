#ifndef __DMMR_PARSER_H__
#define __DMMR_PARSER_H__

#include <funcs.h>

struct dmmr_parser {
    int (*load)(void);
    void (*run)(void);
    void (*stop)(void);
};

struct dmmr_parser* new_dmmr_parser(
    struct cfg_daemon_server*,
    struct cfg_server_server*
);

#endif