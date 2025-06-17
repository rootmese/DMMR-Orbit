#ifndef __DMMR_PARSER_H__
#define __DMMR_PARSER_H__

#include <defs.h>


struct dmmr_parser {
    int (*load)(void);
    void (*run)(void);
    void (*stop)(void);
    void (*handle_token)(void (*handle_token)(const char *, const char *));
};

struct dmmr_parser* new_dmmr_parser(struct cfg_daemon_server*);

#endif 
