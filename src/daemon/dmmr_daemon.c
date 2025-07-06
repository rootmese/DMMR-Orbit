#include <stdio.h>
#include <string.h>

#include <defs.h>

#include <dmmr_daemon.h>
#include<dmmr_server.h>

#include <rb_log.h>
#include <rb_trycatch.h>
#include <rb_exceptions.h>

static struct dmmr_server *srv = 0;

static struct cfg_daemon_server *cfg;

static int reload_pipe[2] = {EOF, EOF};

static struct dmmr_daemon *this = 0;

static void daemonize(try_catch_t *ctx) {
    int ret;
    pid_t pid;
    pid = fork();
    switch (pid) {
        case 0:
            break;
        case EOF:
            RB_THROW_ERRNO((*ctx), errno);
            break; /* Stupid Break :P */
        default:
            exit(0);
    }
    ret = setsid();
    if (ret == EOF)
        RB_THROW_ERRNO((*ctx), errno);
    pid = fork();
    switch (pid) {
        case 0:
            umask(0);
            chdir("/");
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            int fd = open("/dev/null", O_RDWR);
            if (fd >= 0) {
                dup2(fd, STDIN_FILENO);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                if (fd > STDERR_FILENO)
                    close(fd);
            }
            break;
        case EOF:
            RB_THROW_ERRNO((*ctx), errno);
            break; /* Stupid Break :P */
        default:
            exit(0); // pai sai
    }
}

static int daemon_run(void){
    return ((this && srv) ? (srv->run()) : (EOF));
}

static int daemon_start(void) {
    if (!this)
        return EOF;
    try_catch_t ctx;
    RB_TRY(ctx) {
        daemonize(&ctx);
        srv = new_dmmr_server(cfg);
        if (!srv)
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        rb_log_push(LOG_INFO, "DMMR orbiting.", __func__, __LINE__);
        return 0;
    }
    RB_CATCH(ctx) {
        fprintf(stderr, "[FATAL] new_dmmr_daemon fail: %s\n", rb_exc_message(ctx.exception_code));
        return 0;
    }
}
static int daemon_stop(void) {
    return kill(getpid(), SIGTERM);
}

static int reload_config(void) {
    return 0;
}

struct dmmr_daemon* new_dmmr_daemon(struct cfg_daemon_server* __cfg) {
    try_catch_t ctx;
    RB_TRY(ctx) {
        if (!__cfg)
            RB_THROW(ctx, RB_EXC_NULL_CONFIG);

        struct dmmr_daemon* dmn = calloc(1, sizeof(struct dmmr_daemon));
        if (!dmn)
            RB_THROW(ctx, RB_EXC_ALLOC_FAIL);

        dmn->start  = daemon_start;
        dmn->stop   = daemon_stop;
        dmn->reload = reload_config;
        dmn->run    = daemon_run;
        this = dmn;
        cfg  = __cfg;

        rb_log_push(LOG_INFO, "Daemon configurado", __func__, __LINE__);
        return dmn;
    }
    RB_CATCH(ctx) {
        rb_log_push(LOG_ERR, rb_exc_message(ctx.exception_code), __func__, __LINE__);
        return 0;
    }
}

// TODO criar aqui a manipulação de signals
void free_dmmr_daemon(void) {
    if(this){
        if(srv)
            srv->stop();
        free(this);
    }
}
