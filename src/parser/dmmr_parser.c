#include <dmmr_parser.h>
#include <crc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

extern FILE* yyin;
extern int yylex();

static uint8_t run = 0;

static unsigned long last_crc = 0;
static struct dmmr_parser *this = 0;

static struct cfg_daemon_server *cfg_daemon = 0;

static void (*handle_token)(const char *, const char *) = 0;

static int parser_load_config(void) {

    FILE *f = fopen((char*)(cfg_daemon->cfg_file), "r");
    if (!f) {
        perror("[parser] erro ao abrir conf");
        return -1;
    }

    generate_crc32_table();
    last_crc = crc32_file(cfg_daemon->cfg_file);

    yyin = f;
    yylex();
    fclose(f);

    if (handle_token)
        handle_token("config_reload", (char*)(cfg_daemon->cfg_file));

    return 0;
}

static void parser_reload_config(void) {
    do {
        uint32_t crc = crc32_file(cfg_daemon->cfg_file);
        if (crc && crc != last_crc) {
            last_crc = crc;
            parser_load_config();
        }
        sleep(1);
    }while(run);
}

static void parser_stop_config(void) {
    run = 0;
    sleep(1);
}

static inline void dmmr_parser_set_handle_token(void (*handle_token)(const char *, const char *)) {
    if (this)
        handle_token = handle_token;
}

struct dmmr_parser* new_dmmr_parser(struct cfg_daemon_server *__cfg_daemon) {
    struct dmmr_parser* p = (struct dmmr_parser*)calloc(1, sizeof(struct dmmr_parser));
    if (p && __cfg_daemon) {
        cfg_daemon = __cfg_daemon;
        p->load = parser_load_config;
        p->run = parser_reload_config;
        p->stop = parser_stop_config;
        p->handle_token = dmmr_parser_set_handle_token;
        run = !0;
        this = p;
        return p;
    }
    return 0;
}
