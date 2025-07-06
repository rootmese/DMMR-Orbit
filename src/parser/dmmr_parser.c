#include <dmmr_parser.h>
#include <console.h>

#include <rb_log.h>
#include <rb_trycatch.h>

static uint8_t run = 0;
static unsigned long last_crc = 0;
static struct dmmr_parser *this = 0;
static struct cfg_daemon_server *cfg_daemon = 0;
static struct cfg_server_server *cfg_server = 0;

static int parser_load_config(void) {
    if (!cfg_daemon || !cfg_server)
        return EOF;
    int ret;
    ret = console_setup(cfg_daemon->cfg_file, cfg_server);
    if (ret)
        return EOF;
    ret = console_run();
    if (ret)
        return EOF;
    generate_crc32_table();
    last_crc = crc32_file(cfg_daemon->cfg_file);
    return 0;
}

static void parser_reload_config(void) {
    int ret;
    uint32_t crc = crc32_file(cfg_daemon->cfg_file);
    if (crc && crc != last_crc) {
        last_crc = crc;
        console_cleanup();
        ret = console_setup(cfg_daemon->cfg_file, cfg_server);
        if (!ret)
            console_run();
    }
}

static void parser_stop_config(void) {
    run = 0;
    console_cleanup();
}

struct dmmr_parser* new_dmmr_parser(
    struct cfg_daemon_server *__cfg_daemon,
    struct cfg_server_server *__cfg_server
) {
    try_catch_t ctx;
    RB_TRY(ctx) {
        // Validação dos parâmetros
        if (!__cfg_daemon || !__cfg_server)
            RB_THROW(ctx, RB_EXC_NULL_CONFIG);

        // Alocação do parser
        struct dmmr_parser* p = calloc(1, sizeof(struct dmmr_parser));
        if (!p)
            RB_THROW(ctx, RB_EXC_ALLOC_FAIL);

        // Configuração básica
        cfg_daemon = __cfg_daemon;
        cfg_server = __cfg_server;
        this = p;
        run = !0;

        // Atribuição das funções
        p->load = parser_load_config;
        p->run = parser_reload_config;
        p->stop = parser_stop_config;

        rb_log_push(LOG_INFO, "Parser configurado", __func__, __LINE__);
        return p;
    }
    RB_CATCH(ctx) {
        rb_log_push(LOG_ERR, rb_exc_message(ctx.exception_code), __func__, __LINE__);
        return 0;
    }
}