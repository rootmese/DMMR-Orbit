#include <stdio.h>
#include <string.h>

#include <__vcpy.h>
#include <__mset.h>

#include <rb_log.h>

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

static int server_start(void){
    if(this){
        int ret;
        parser = new_dmmr_parser(cfg_daemon, &cfg);
        if(!parser)
            return EOF;
        ret = parser->load();
        if(ret)
            return EOF;
        rb_log_init(&cfg);
        circle_buffer = new_circle_buffer(&cfg);
        if(!circle_buffer)
            return EOF;
        ret = circle_buffer->start();
        if(ret)
            return EOF;
        sock = new_dmmr_socket(circle_buffer, &cfg);
        if(!sock)
            return EOF;
        scheduler = new_dmmr_scheduler(sock, &cfg);
        if(!scheduler)
            return EOF;
        ret = scheduler->start();
        if(ret)
            return EOF;
        session_manager = new_session_connection_manager(circle_buffer, scheduler, sock, &cfg);
        if(!session_manager)
            return EOF;
        plugin = new_dmmr_plugin(session_manager, &cfg);
        if(!plugin)
            return EOF;
        plugin->load(); // server proc
        sleep(1);
    }
    return 0;
}

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server *__daemon_cfg) {
    struct dmmr_server* srv = calloc(1, sizeof(struct dmmr_server));
     if (!srv)
        return 0;
    cfg_daemon = __daemon_cfg; 
    srv->run = server_run;
    srv->start = server_start;
    srv->stop = server_stop;
    this = srv;
    return srv;
}

void free_dmmr_server(struct dmmr_server* this) {
    if (this)
        free(this);
}