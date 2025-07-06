#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <dmmr_daemon.h>
#include <rb_log.h>

static struct cfg_daemon_server cfg;

static volatile int run = 0;

static void handle_signal(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        fprintf(stderr, "\n[SIGNAL] Encerrando foreground (recebido sinal %d)\n", signo);
        run = 0;
    }
}

static void process_args(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "d:f:")) != EOF) {
        switch (opt) {
            case 'f':
                strlcpy((char*)cfg.cfg_file, optarg, sizeof(cfg.cfg_file) - 1);
                struct stat sb;
                if (stat(cfg.cfg_file, &sb) != 0) {
                    perror("Falha ao acessar arquivo de configuração");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                cfg.daemonize = atoi(optarg) & 0xFF;
                break;
            default:
                fprintf(stderr, "Uso: %s [-f arquivo.conf] [-d 0|1]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int run_foreground(void){
    if (signal(SIGINT, handle_signal) == SIG_ERR ||
        signal(SIGTERM, handle_signal) == SIG_ERR) {
        fprintf(stderr, "[ERRO] Falha ao configurar handler de sinal\n");
        return EOF;
    }
    struct dmmr_server* server = new_dmmr_server(&cfg);
    if (!server) {
        fprintf(stderr, "Falha ao criar o servidor: %d (%s)\n", errno, strerror(errno));
        return EOF;
    }
    server->start();
    // Rodar até receber sinal
    rb_log_push(rb_log_level_info, "[INFO] Servidor rodando em foreground. Pressione Ctrl+C para sair...", __FUNCTION__, __LINE__);
    run = !0;
   do {
        int status = server->run(); // Idealmente deve ser não-bloqueante ou loop com timeout
        if (status != 0) {
            fprintf(stderr, "[ERRO] server->run() retornou %d\n", status);
            break;
        }
    } while (run);
    fprintf(stderr, "[INFO] Parando servidor...\n");
    server->stop();
    free_dmmr_server(server);
    fprintf(stderr, "[INFO] Encerrado.\n");
    rb_log_flush();
    return 0;
}


int main(int argc, char** argv) {
    __mset(&cfg, 0, sizeof(struct cfg_daemon_server));
    process_args(argc, argv);
    if(!(cfg.daemonize))
        return run_foreground();
    else{
        struct dmmr_daemon* daemon = new_dmmr_daemon(&cfg);
        if (!daemon) {
            fprintf(stderr, "Falha ao criar o daemon\n");
            exit(EXIT_FAILURE);
        }
        if (daemon->start() != 0) {
            fprintf(stderr, "Falha ao iniciar o daemon\n");
            exit(EXIT_FAILURE);
        }
        else{
            int ret = daemon->run();
            rb_log_flush();
            return ret;
        }
    }
}
