#ifndef __DMMR_PARSER_H__
#define __DMMR_PARSER_H__

#include <defs.h>

#define TOKEN_IDENT      100
#define TOKEN_NUMBER     101
#define TOKEN_COMMA      102
#define TOKEN_SEMICOLON  103
#define TOKEN_SYMBOL     104
#define TOKEN_STRING     105


struct dmmr_parser {
    int (*load)(void);
    void (*run)(void);
    void (*stop)(void);
};

struct dmmr_parser* new_dmmr_parser(struct cfg_daemon_server*);

#endif 
