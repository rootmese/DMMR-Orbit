#include <stdio.h>
#include <string.h>

#include <__vcpy.h>
#include <__mset.h>

#include <dmmr_server.h>
#include <dmmr_parser.h>
#include <dmmr_plugin.h>

static struct cfg_server_server cfg;

static struct cfg_daemon_server *cfg_daemon;

enum {
    NONE,
    SET_SCHEDULER_PREEMPTIVE_DEADLINE,
    SET_SLEEP_TIME,
    SET_SESSION_SIZE,
    SET_CIRCLE_BUFFER_SIZE,
    SET_MAX_PORTS,
    SET_REAL_TIME_DEAD_LINE,
    SET_REAL_TIME_USER_DEFINED,
    SET_TRUNK_ACCEPT_URI,
    SET_TRUNK_DISPATCH_URI
} state = NONE;

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

int yylex(void);

static void handle_signal(int sig) {
    if (sig == SIGUSR1)
        reload = 1;
    else if (sig == SIGINT || sig == SIGTERM) {
        reload = 0;
        run = 0;
    }
}

static void server_handle_token(const char* type, const char* value) {
    // static int parsing_ports = 0;
    // static unsigned start_port = 0;
    // static unsigned expect_range = 0;

/*    if (!strcmp(type, "PORTS")) {
        parsing_ports = 1;
        parser->num_ports = 0;
        start_port = 0;
        expect_range = 0;
        return;
    }*/
/*
    if (parsing_ports) {
        if (!strcmp(type, "NUMBER")) {
            unsigned port = (unsigned)atoi(value);
            if (expect_range) {
                if (start_port && port > start_port) {
                    for (unsigned p = start_port; p <= port; p++) {
                        if (parser->num_ports < MAX_PORTS)
                            parser->ports[parser->num_ports++] = p;
                    }
                }
                start_port = 0;
                expect_range = 0;
            } else if (start_port) {
                if (parser->num_ports < MAX_PORTS)
                    parser->ports[parser->num_ports++] = start_port;
                start_port = port;
            } else {
                start_port = port;
            }
        } else if (!strcmp(type, "DASH")) {
            expect_range = 1;
        } else if (!strcmp(type, "COMMA")) {
            if (start_port && parser->num_ports < MAX_PORTS)
                parser->ports[parser->num_ports++] = start_port;
            start_port = 0;
        } else if (!strcmp(type, "SEMICOLON")) {
            if (start_port && parser->num_ports < MAX_PORTS)
                parser->ports[parser->num_ports++] = start_port;
            parsing_ports = 0;
        }
        return;
    }*/

    // Novo: interpretar configurações
    if (!strcmp(type, "KEYWORD")) {
        if (!strcmp(value, "SESSION_SIZE")) state = SET_SESSION_SIZE;
        else if (!strcmp(value, "SLEEP_TIME")) state = SET_SLEEP_TIME;
        else if (!strcmp(value, "CIRCLE_BUFFER_SIZE")) state = SET_CIRCLE_BUFFER_SIZE;
        else if (!strcmp(value, "MAX_PORTS")) state = SET_MAX_PORTS;
        else if (!strcmp(value, "REAL_TIME_DEAD_LINE")) state = SET_REAL_TIME_DEAD_LINE;
        else if (!strcmp(value, "REAL_TIME_USER_DEFINED")) state = SET_REAL_TIME_USER_DEFINED;
        else if (!strcmp(value, "TRUNK_ACCEPT_URI")) state = SET_TRUNK_ACCEPT_URI;
        else if (!strcmp(value, "TRUNK_DISPATCH_URI")) state = SET_TRUNK_DISPATCH_URI;
        else if (!strcmp(value, "SCHEDULER_PREEMPTIVE_DEADLINE")) state = SET_SCHEDULER_PREEMPTIVE_DEADLINE;
        else state = NONE;
    }
    else if (!strcmp(type, "NUMBER") || !strcmp(type, "STRING")) {
        switch (state) {
            case SET_SCHEDULER_PREEMPTIVE_DEADLINE:
                cfg.scheduler_preemptive_deadline = strtoull(value, 0, 10); break;
            case SET_SLEEP_TIME:
                cfg.sleep_time = (uint16_t)atoi(value); break;
            case SET_SESSION_SIZE:
                cfg.session_size = (uint16_t)atoi(value); break;
            case SET_CIRCLE_BUFFER_SIZE:
                cfg.circle_buffer_size = (uint32_t)atoi(value); break;
            case SET_MAX_PORTS:
                cfg.max_ports = (uint16_t)atoi(value); break;
            case SET_REAL_TIME_DEAD_LINE:
                cfg.real_time_dead_line = strtoull(value, 0, 10); break;
            case SET_REAL_TIME_USER_DEFINED:
                cfg.real_time_user_defined = strtoull(value, 0, 10); break;
            case SET_TRUNK_ACCEPT_URI:
                strlcpy((char*)cfg.trunk_accept_uri, value, sizeof(cfg.trunk_accept_uri)); break;
            case SET_TRUNK_DISPATCH_URI:
                strlcpy((char*)cfg.trunk_dispatch_uri, value, sizeof(cfg.trunk_dispatch_uri)); break;
            default:
                break;
        }
    }
    state = NONE;
}


static void server_stop(void) {
}

static int server_run(void){
    if(this){
        int ret;
        signal(SIGUSR1, handle_signal);
        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);

        parser = new_dmmr_parser(cfg_daemon);
        if(!parser)
            goto return_error;

        parser->handle_token(server_handle_token);
        ret = parser->load();
        if(ret)
            goto return_error;

        circle_buffer = new_circle_buffer(&cfg);
        if(!circle_buffer)
            goto return_error;

        ret = circle_buffer->start();
        if(ret)
            goto return_error;
    
        sock = new_dmmr_socket(circle_buffer, &cfg);
        if(!sock)
            goto return_error;

        scheduler = new_dmmr_scheduler(sock, &cfg);
        if(!scheduler)
            goto return_error;

        ret = scheduler->start();
        if(ret)
            goto return_error;

        session_manager = new_session_connection_manager(circle_buffer, scheduler, sock, &cfg);
        if(!session_manager)
            goto return_error;

        plugin = new_dmmr_plugin(session_manager, &cfg);
        if(!plugin)
            goto return_error;

        plugin->load(); // server proc

        parser->run(); // main loop

        return 0;

        return_error:
        if(parser)
            free(parser);
        if(circle_buffer){
            circle_buffer->stop();
            free(circle_buffer);
        }
        if(sock)
            free(sock);
        if(scheduler){
            scheduler->stop();
            free(scheduler);
        }
        if(session_manager)
            free(session_manager);
        if(plugin)
            free(plugin);
        return EOF;
    }
}

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server *__daemon_cfg) {
    struct dmmr_server* srv = calloc(1, sizeof(struct dmmr_server));
     if (!srv)
        return 0;
     __mset(&cfg, 0, sizeof(struct cfg_server_server));
    srv-> cfg = &cfg;
    srv->cfg_daemon = __daemon_cfg;
    srv->run = server_run;
    srv->stop = server_stop;
    this = srv;
    return srv;
}

void free_dmmr_server(struct dmmr_server* this) {
    if (this)
        free(this);
}