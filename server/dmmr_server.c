#include <dmmr_server.h>


extern FILE* yyin;
int yylex(void);

static struct dmmr_server *this;


static void server_handle_token(struct dmmr_parser* parser, const char* type, const char* value) {
    static int parsing_ports = 0;
    static unsigned start_port = 0;
    static unsigned expect_range = 0;
    
    // Estado inicial: não estamos processando portas
    if (!parsing_ports && strcmp(type, "PORTS") == 0) {
        parsing_ports = 1;
        parser->num_ports = 0;
        start_port = 0;
        expect_range = 0;
        return;
    }
    
    // Se não estamos no modo ports, ignorar tokens
    if (!parsing_ports) return;
    
    // Máquina de estados para processamento de portas
    if (strcmp(type, "NUMBER") == 0) {
        unsigned port = (unsigned)atoi(value);
        
        if (expect_range) {
            // Processar faixa de portas
            if (start_port && port > start_port) {
                for (unsigned p = start_port; p <= port; p++) {
                    if (parser->num_ports < MAX_PORTS) {
                        parser->ports[parser->num_ports++] = p;
                    }
                }
            }
            start_port = 0;
            expect_range = 0;
        } else if (start_port) {
            // Finalizar porta única pendente
            if (parser->num_ports < MAX_PORTS) {
                parser->ports[parser->num_ports++] = start_port;
            }
            start_port = port;
        } else {
            // Nova porta inicial
            start_port = port;
        }
    }
    else if (strcmp(type, "DASH") == 0) {
        expect_range = 1;
    }
    else if (strcmp(type, "COMMA") == 0 && start_port) {
        // Finalizar porta única
        if (parser->num_ports < MAX_PORTS) {
            parser->ports[parser->num_ports++] = start_port;
        }
        start_port = 0;
    }
    else if (strcmp(type, "SEMICOLON") == 0) {
        // Finalizar bloco de portas
        if (start_port) {
            if (parser->num_ports < MAX_PORTS) {
                parser->ports[parser->num_ports++] = start_port;
            }
        }
        parsing_ports = 0;
        start_port = 0;
        expect_range = 0;
    }
    else {
        // Token inesperado - resetar parsing
        parsing_ports = 0;
        start_port = 0;
        expect_range = 0;
    }
}

static void server_run(struct dmmr_server* this) {
    if (this) {
        const char* cfg_path = (const char*)this->cfg_daemon.cfg_file;
        uint32_t last_crc = 0;

        generate_crc32_table();

         last_crc = crc32_file(cfg_path);

        if (!last_crc) {
            perror("Erro ao calcular CRC do arquivo de configuração");
            return;
        }

        FILE* f = fopen(cfg_path, "r");
        if (!f) {
            perror("Erro ao abrir arquivo de configuração");
            return;
        }

        yyin = f;
        yylex();
        fclose(f);

        do {
            uint32_t current_crc = crc32_file(cfg_path);
            if (current_crc){
                if (!(current_crc == last_crc)) {
                    printf("[server] Arquivo de configuração alterado. Recarregando...\n");

                    last_crc = current_crc;

                    f = fopen(cfg_path, "r");
                    if (!f) {
                        perror("Erro ao reabrir arquivo de configuração");
                        continue;
                    }

                    yyin = f;
                    yylex();
                    fclose(f);
                }
            }
            sleep(0x01); // monitor loop
        }while(~0);
    }
}


static void server_shutdown(struct dmmr_server* this) {
    if (this && this->cb) {
        pthread_cancel(this->cb->fifo_thread);
        pthread_join(this->cb->fifo_thread, 0);
    }
}

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server *__daemon_cfg) {
    struct dmmr_server* srv = calloc(1, sizeof(struct dmmr_server));
    if (!srv) return 0;

    // Copiar configurações
    memcpy(&srv->cfg_daemon, __daemon_cfg, sizeof(struct cfg_daemon_server));

    // Inicializar componentes
    srv->cb = new_circle_buffer(10);
    srv->parser = new_dmmr_parser();
    struct dmmr_scheduler*
    
    // Configurar o parser
    if (srv->parser) {
        srv->parser->context = srv;
        srv->parser->handle_token = server_handle_token;
    }
    
    // Configurar métodos
    srv->run = server_run;
    srv->shutdown = server_shutdown;
    
    return srv;
}

void free_dmmr_server(struct dmmr_server* this) {
    if (!this) return;
    
    // Liberar recursos do circle_buffer (se houver função, implemente)
    // if (this->cb) free_circle_buffer(this->cb);
    if (this->parser) free(this->parser);
    free(this);
}