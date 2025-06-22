#include <stdio.h>
#include <string.h>

#include <defs.h>

#include <dmmr_daemon.h>
#include<dmmr_server.h>

static struct dmmr_server *srv = 0;

static struct cfg_daemon_server *cfg;

static int reload_pipe[2] = {-1, -1};

static struct dmmr_daemon *this = 0;

static void daemonize(void) {
    pid_t pid;
    pid = fork();
    switch (pid) {
        case 0:
            break;
        case EOF:
            fprintf(stderr, "Fork error: %s\n", strerror(errno));
            exit(EOF);
        default:
            exit(0);
    }
    if (setsid() < 0)
        exit(EOF);
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
            fprintf(stderr, "Fork error: %s\n", strerror(errno));
            exit(EOF);
        default:
            exit(0); // pai sai
    }
}

static int daemon_run(void){
    return ((this && srv) ? (srv->run()) : (EOF));
}

static int daemon_start(void) {
    if(this){
        daemonize();
        srv = new_dmmr_server(cfg);
        if(!srv)
            return EOF;
        return 0;
    }
    else
        return EOF;
}

static int daemon_stop(void) {
    return kill(getpid(), SIGTERM);
}

static int reload_config(void) {
    return 0;
}

struct dmmr_daemon* new_dmmr_daemon(struct cfg_daemon_server* __cfg) {
    struct dmmr_daemon* dmn = calloc(1, sizeof(struct dmmr_daemon));
    if (dmn && __cfg){
        dmn->start = daemon_start;
        dmn->stop = daemon_stop;
        dmn->reload = reload_config;
        dmn->run = daemon_run;
        this = dmn;
        cfg = __cfg;
        return dmn;
    }
    else
        return 0;
}

// TODO criar aqui a manipulação de signals
void free_dmmr_daemon(void) {
    if(this){
        if(srv)
            srv->stop();
        free(this);
    }
}
