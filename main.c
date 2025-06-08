#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <string.h>
#include <dmmr_server.h>

static void process_args(int argc, char **argv, struct cfg_daemon_server *cfg) {
    int opt;
    memset(cfg, 0, sizeof(struct cfg_daemon_server));
    while ((opt = getopt(argc, argv, "f:")) != EOF) {
        switch (opt) {
            case 'f': // Arquivo de configuração
                strlcpy((char*)cfg->cfg_file, optarg, sizeof(cfg->cfg_file) - 1);
                struct stat sb;
                if (stat(cfg->cfg_file, &sb) != 0) {
                    perror("Falha ao acessar arquivo de configuração");
                    exit(EXIT_FAILURE);
                }
                break;
            default: // Opção inválida
                fprintf(stderr, "Uso: %s [-f arquivo.conf]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv) {
    int pid = fork();
    switch (pid) {
        case 0:
        {
            struct cfg_daemon_server cfg;
            process_args(argc, argv, &cfg);
            struct dmmr_server* server = new_dmmr_server(&cfg);
            if (!server) {
                fprintf(stderr, "Falha ao criar o servidor\n");
                exit(EXIT_FAILURE);
            }
            server->run(server);
            free_dmmr_server(server);
            break;
        }
        case EOF:
            perror("Erro no fork()");
            exit(EXIT_FAILURE);
            break; /* Stupid Break:P */
        default:
        {
            char output_file[0x200];
            int l = snprintf(output_file, sizeof(output_file), "/var/run/%s.pid", *argv);
            if (l < 0 || l >= sizeof(output_file)) {
                perror("Erro ao formatar nome do arquivo PID");
               exit(EXIT_FAILURE);
            }
            FILE *output_file_ptr = fopen(output_file, "w");
            if (!output_file_ptr) {
                perror("Erro ao criar arquivo de saída");
                exit(EXIT_FAILURE);
            }
            else {
                fprintf(output_file_ptr, "%d", pid);
                fclose(output_file_ptr);
            }
            wait(0);
            break;
        }
    }
    return 0;
}