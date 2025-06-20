#include <stdio.h>
#include <string.h>

#include <__vcpy.h>
#include <__mset.h>

#include <dmmr_server.h>
#include <dmmr_parser.h>
#include <dmmr_plugin.h>

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
    if(plugin)
            free(plugin);
    if(session_manager)
            free(session_manager);
    if(scheduler){
        scheduler->stop();
        free(scheduler);
    }
    if(sock)
        free(sock);
    if(circle_buffer){
        circle_buffer->stop();
        free(circle_buffer);
    }
    if(parser)
        free(parser);
    return 0;
}

static void server_stop(void) {
}

static int server_run(void){
    if(this){
        int ret;
        signal(SIGUSR1, handle_signal);
        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);

        LOG();
        parser = new_dmmr_parser(cfg_daemon, &cfg);
        if(!parser)
            return EOF;
        
        LOG();
        ret = parser->load();
        if(ret)
            return EOF;

        LOG();
        circle_buffer = new_circle_buffer(&cfg);
        if(!circle_buffer)
            return EOF;

        LOG();
        ret = circle_buffer->start();
        if(ret)
            return EOF;

        LOG();    
        sock = new_dmmr_socket(circle_buffer, &cfg);
        if(!sock)
            return EOF;

        LOG();
        scheduler = new_dmmr_scheduler(sock, &cfg);
        if(!scheduler)
            return EOF;

        LOG();
        ret = scheduler->start();
        if(ret)
            return EOF;

        LOG();
        session_manager = new_session_connection_manager(circle_buffer, scheduler, sock, &cfg);
        if(!session_manager)
            return EOF;

        LOG();
        plugin = new_dmmr_plugin(session_manager, &cfg);
        if(!plugin)
            return EOF;

        LOG();
        plugin->load(); // server proc
        sleep(1);
        return server_proc();

    }
}

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server *__daemon_cfg) {
    struct dmmr_server* srv = calloc(1, sizeof(struct dmmr_server));
     if (!srv)
        return 0;
    cfg_daemon = __daemon_cfg; 
    srv->run = server_run;
    srv->stop = server_stop;
    this = srv;
    return srv;
}

void free_dmmr_server(struct dmmr_server* this) {
    if (this)
        free(this);
}