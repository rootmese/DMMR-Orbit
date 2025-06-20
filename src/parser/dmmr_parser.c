#include <dmmr_parser.h>
#include <console.h>
#include <crc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint8_t run = 0;
static unsigned long last_crc = 0;
static struct dmmr_parser *this = 0;
static struct cfg_daemon_server *cfg_daemon = 0;
static struct cfg_server_server *cfg_server = 0;

static int parser_load_config(void) {
    if (!cfg_daemon || !cfg_server) return EOF;
    
    // Carregar configuração inicial
    if (console_setup(cfg_daemon->cfg_file, cfg_server) != 0) {
        return EOF;
    }
    
    if (console_run() != 0) {
        return EOF;
    }
    
    // Calcular CRC para futuras verificações
    generate_crc32_table();
    last_crc = crc32_file(cfg_daemon->cfg_file);
    
    return 0;
}

static void parser_reload_config(void) {
    do {
        LOG();
        uint32_t crc = crc32_file(cfg_daemon->cfg_file);
        if (crc && crc != last_crc) {
            last_crc = crc;
            
            // Recarregar configuração
            console_cleanup();
            if (console_setup(cfg_daemon->cfg_file, cfg_server) == 0) {
                console_run();
            }
        }
        LOG();
        sleep(1);
    } while(run);
    LOG();
}

static void parser_stop_config(void) {
    run = 0;
    console_cleanup();
}

struct dmmr_parser* new_dmmr_parser(
    struct cfg_daemon_server *__cfg_daemon,
    struct cfg_server_server *__cfg_server
) {
    struct dmmr_parser* p = calloc(1, sizeof(struct dmmr_parser));
    if (p && __cfg_daemon && __cfg_server) {
        cfg_daemon = __cfg_daemon;
        cfg_server = __cfg_server;
        
        p->load = parser_load_config;
        p->run = parser_reload_config;
        p->stop = parser_stop_config;
        
        run = !0;
        this = p;
        
        return p;
    }
    return NULL;
}