#ifndef __DMMR_PARSER_H__
#define __DMMR_PARSER_H__

#include <stdio.h>

// Tipos de tokens reconhecidos
typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_IPADDR,
    TOKEN_EQUALS
} dmmt_token_type;

// Estrutura de um token genérico
struct dmmt_token {
    dmmt_token_type type;
    char value[0x100]; // tamanho arbitrário pra MVP
};

// Pseudo-interface de parser
struct dmmr_parser {

    unsigned char *cfg_file;
    void (*init)(struct DMMR_parser* self);
    server_handle_token handle_token;
};

// Função para criar uma instância genérica de parser
struct DMMR_parser* new_dmmr_parser(void);

#endif // __DMMR_PARSER_H__
