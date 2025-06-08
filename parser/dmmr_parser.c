#include <dmmr_parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

extern FILE* yyin;
extern int yylex();

static unsigned long last_crc = 0;

static void parser_run(struct dmmr_parser* this) {
    if (this)
    {
        generate_crc32_table();
        last_crc = crc32_file((const char*)this->cfg_file);

        if (!last_crc) {
            perror("[parser] Erro ao calcular CRC inicial");
            return;
        }

        FILE* f = fopen(this->config_path, "r");
        if (!f) {
            perror("[parser] Erro ao abrir arquivo de configuração");
            return;
        }

        yyin = f;
        yylex();
        fclose(f);

        // Aqui você poderia montar uma estrutura cfg_server ou algo do tipo,
        // e chamar o callback para avisar que nova config foi carregada
        if (this->handle_token) {
            // Exemplo: chamar callback com token fictício (ou dados relevantes)
            this->handle_token(this, "config_reload", this->config_path);
        }

            uint32_t current_crc = crc32_file(this->config_path);
            if (current_crc && current_crc != last_crc) {
                printf("[parser] Detecção de alteração no arquivo. Recarregando...\n");

                last_crc = current_crc;

                f = fopen(this->config_path, "r");
                if (!f) {
                    perror("[parser] Erro ao reabrir arquivo de configuração");
                    continue;
                }

                yyin = f;
                yylex();
                fclose(f);

                if (this->handle_token) {
                    // Notifica nova configuração carregada
                    this->handle_token(this, "config_reload", this->config_path);
                }
            }
    }
}

// Cria e retorna um parser com a função de run e ponteiro de callback
struct dmmr_parser* new_dmmr_parser(struct cfg_daemon_server *__cfg_daemon) {
    struct dmmr_parser* p = calloc(1, sizeof(struct dmmr_parser));
    if (p){

    p->run = parser_run;
    p->last_crc = 0;
    p->handle_token = __cfg_daemon->server_handle_token;

    return p;
    }
    else
        return 0;
}
