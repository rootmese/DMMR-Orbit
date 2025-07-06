

#include <dmmr_server.h>
#include <dmmr_parser.h>
#include <dmmr_plugin.h>
#include <rb_log.h>
#include <rb_trycatch.h>
#include <rb_log.h>

static struct cfg_server_server cfg;

static struct cfg_daemon_server *cfg_daemon;

static struct dmmr_parser *parser = 0;

static struct dmmr_circle_buffer *circle_buffer = 0;

static struct dmmr_scheduler *scheduler = 0;

static struct dmmr_socket *sock = 0;

static struct dmmr_session_connection_manager *session_manager = 0;

static struct dmmr_plugin *plugin = 0;

static struct dmmr_server *this = 0;

static volatile uint8_t run = 0;

static volatile uint8_t reload = 0;


extern FILE* yyin;

extern int yylex(void);

static void handle_signal(int sig) {
    switch(sig){
        case SIGUSR1:
            reload = 1;
            break;
        case SIGINT:
        case SIGTERM:
            reload = 0;
            run = 0;
            break;
        default:
            break; // Stupid break :P
    }
}

int server_proc(void){

    (void)(parser->run());
    return 0;
}

static int server_run(void){
    if(this){
        sleep(1);
        return server_proc();

    }
}

static void server_stop(void){
    if(plugin)
        free(plugin);
    if(session_manager)
        free(session_manager);
    if(scheduler){
        scheduler->stop();
        sleep(1);
        free(scheduler);
    }
    if(sock)
        free(sock);
    if(circle_buffer){
        circle_buffer->stop();
        sleep(1);
        free(circle_buffer);
    }
    if(parser)
        free(parser);
    rb_log_shutdown();
}

static int server_start(void) {
    if (!this) {
        rb_log_push(LOG_ERR, "Ponteiro 'this' nulo", __func__, __LINE__);
        return EOF;
    }

    try_catch_t ctx;
    RB_TRY(ctx) {
        // 1. Inicializa Parser
        parser = new_dmmr_parser(cfg_daemon, &cfg);
        if (!parser) {
            rb_log_push(LOG_ERR, "Falha ao criar parser", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        if (parser->load() != 0) {
            rb_log_push(LOG_ERR, "Falha ao carregar parser", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        // 2. Inicializa Sistema de Logs
        rb_log_init(&cfg);

        // 3. Cria e inicia Circle Buffer
        circle_buffer = new_circle_buffer(&cfg);
        if (!circle_buffer) {
            rb_log_push(LOG_ERR, "Falha ao criar circle buffer", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        if (circle_buffer->start() != 0) {
            rb_log_push(LOG_ERR, "Falha ao iniciar circle buffer", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        // 4. Configura Socket
        sock = new_dmmr_socket(circle_buffer, &cfg);
        if (!sock) {
            rb_log_push(LOG_ERR, "Falha ao criar socket", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        // 5. Inicializa Scheduler
        scheduler = new_dmmr_scheduler(sock, &cfg);
        if (!scheduler) {
            rb_log_push(LOG_ERR, "Falha ao criar scheduler", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        if (scheduler->start() != 0) {
            rb_log_push(LOG_ERR, "Falha ao iniciar scheduler", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        // 6. Gerenciador de Sessões
        session_manager = new_session_connection_manager(circle_buffer, scheduler, sock, &cfg);
        if (!session_manager) {
            rb_log_push(LOG_ERR, "Falha ao criar gerenciador de sessões", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        // 7. Plugin
        plugin = new_dmmr_plugin(session_manager, &cfg);
        if (!plugin) {
            rb_log_push(LOG_ERR, "Falha ao carregar plugin", __func__, __LINE__);
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        }

        plugin->load();  // Carrega o plugin (pode ser bloqueante)
        sleep(1);  // Espera inicialização

        rb_log_push(LOG_INFO, "Servidor iniciado com sucesso", __func__, __LINE__);
        return 0;
    }
    RB_CATCH(ctx) {
        // Rollback em caso de falha (desaloca recursos parcialmente criados)
        if (plugin) {
            free(plugin);
            plugin = 0;
        }
        if (session_manager) {
            free(session_manager);
            session_manager = 0;
        }
        if (scheduler) {
            scheduler->stop();
            sleep(1);
            free(scheduler);
            scheduler = 0;
        }
        if (sock) {
            free(sock);
            sock = 0;
        }
        if (circle_buffer) {
            circle_buffer->stop();
            sleep(1);
            free(circle_buffer);
            circle_buffer = 0;
        }
        if (parser) {
            free(parser);
            parser = 0;
        }

        rb_log_push(LOG_CRIT, "Falha crítica ao iniciar servidor", __func__, __LINE__);
        return EOF;
    }
}

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server *__daemon_cfg) {
    try_catch_t ctx;
    RB_TRY(ctx) {
        if (!__daemon_cfg)
            RB_THROW(ctx, RB_EXC_NULL_CONFIG);

        struct dmmr_server* srv = calloc(1, sizeof(struct dmmr_server));
        if (!srv)
            RB_THROW(ctx, RB_EXC_ALLOC_FAIL);

        cfg_daemon = __daemon_cfg; 
        srv->run = server_run;
        srv->start = server_start;
        srv->stop = server_stop;
        this = srv;

        rb_log_push(LOG_INFO, "Servidor DMMR criado", __func__, __LINE__);
        return srv;
    }
    RB_CATCH(ctx) {
        rb_log_push(LOG_ERR, rb_exc_message(ctx.exception_code), __func__, __LINE__);
        return 0;
    }
}
void free_dmmr_server(struct dmmr_server* this) {
    if (this)
        free(this);
}