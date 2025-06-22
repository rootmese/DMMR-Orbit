#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <console.h>
#include <l_tokens.h>
#include <unquote_string.h>

// Define o enum internamente (não precisa estar no header)
typedef enum {
    CONFIG_NONE,
    CONFIG_SCHEDULER_PREEMPTIVE_DEADLINE,
    CONFIG_SLEEP_TIME,
    CONFIG_SESSION_SIZE,
    CONFIG_CIRCLE_BUFFER_SIZE,
    CONFIG_MAX_PORTS,
    CONFIG_REAL_TIME_DEAD_LINE,
    CONFIG_REAL_TIME_USER_DEFINED,
    CONFIG_TRUNK_ACCEPT_URI,
    CONFIG_TRUNK_DISPATCH_URI,
    CONFIG_RB_LOG_LEVEL
} ConfigState;

// Estado interno do console
struct console_state {
    struct cfg_server_server *cfg;
    ConfigState current_setting;
    FILE *input_file;
};

static struct console_state state;

extern FILE *yyin;
int yylex(void);

void handle_token(const char *type, const char *value) {
    if (!state.cfg) return;
    
    if (strcmp(type, "KEYWORD") == 0) {
        if (strcmp(value, "SESSION_SIZE") == 0) 
            state.current_setting = CONFIG_SESSION_SIZE;
        else if (strcmp(value, "SLEEP_TIME") == 0) 
            state.current_setting = CONFIG_SLEEP_TIME;
        else if (strcmp(value, "CIRCLE_BUFFER_SIZE") == 0) 
            state.current_setting = CONFIG_CIRCLE_BUFFER_SIZE;
        else if (strcmp(value, "MAX_PORTS") == 0) 
            state.current_setting = CONFIG_MAX_PORTS;
        else if (strcmp(value, "REAL_TIME_DEAD_LINE") == 0) 
            state.current_setting = CONFIG_REAL_TIME_DEAD_LINE;
        else if (strcmp(value, "REAL_TIME_USER_DEFINED") == 0) 
            state.current_setting = CONFIG_REAL_TIME_USER_DEFINED;
        else if (strcmp(value, "TRUNK_ACCEPT_URI") == 0) 
            state.current_setting = CONFIG_TRUNK_ACCEPT_URI;
        else if (strcmp(value, "TRUNK_DISPATCH_URI") == 0) 
            state.current_setting = CONFIG_TRUNK_DISPATCH_URI;
        else if (strcmp(value, "SCHEDULER_PREEMPTIVE_DEADLINE") == 0) 
            state.current_setting = CONFIG_SCHEDULER_PREEMPTIVE_DEADLINE;
        else if (strcmp(value, "RB_LOG_LEVEL") == 0) 
            state.current_setting = CONFIG_RB_LOG_LEVEL;
    }
    else if (strcmp(type, "NUMBER") == 0) {
        switch (state.current_setting) {
            case CONFIG_SCHEDULER_PREEMPTIVE_DEADLINE:
                state.cfg->scheduler_preemptive_deadline = strtoull(value, NULL, 10);
                break;
            case CONFIG_SLEEP_TIME:
                state.cfg->sleep_time = (uint16_t)atoi(value);
                break;
            case CONFIG_SESSION_SIZE:
                state.cfg->session_size = (uint16_t)atoi(value);
                break;
            case CONFIG_CIRCLE_BUFFER_SIZE:
                state.cfg->circle_buffer_size = (uint32_t)atoi(value);
                break;
            case CONFIG_MAX_PORTS:
                state.cfg->max_ports = (uint16_t)atoi(value);
                break;
            case CONFIG_REAL_TIME_DEAD_LINE:
                state.cfg->real_time_dead_line = strtoull(value, NULL, 10);
                break;
            case CONFIG_REAL_TIME_USER_DEFINED:
                state.cfg->real_time_user_defined = strtoull(value, NULL, 10);
                break;
            case CONFIG_RB_LOG_LEVEL:
                state.cfg->log_level = atoi(value);
                break;
            default:
                break;
        }
        state.current_setting = CONFIG_NONE;
    }
    else if (strcmp(type, "STRING") == 0) {
        switch (state.current_setting) {
            case CONFIG_TRUNK_ACCEPT_URI:
                unquote_string(
                    (char*)state.cfg->trunk_accept_uri,
                    sizeof(state.cfg->trunk_accept_uri),
                    value
                );
                break;
            
            case CONFIG_TRUNK_DISPATCH_URI:
                unquote_string(
                    (char*)state.cfg->trunk_dispatch_uri,
                    sizeof(state.cfg->trunk_dispatch_uri),
                    value
                );
                break;
            
            default:
                break;
        }
        state.current_setting = CONFIG_NONE;
    }
}

int console_setup(const char *file_name, struct cfg_server_server *cfg) {
    if (!file_name || !cfg)
        return EOF;
    memset(&state, 0, sizeof(state));
    state.cfg = cfg;
    state.current_setting = CONFIG_NONE;
    state.input_file = fopen(file_name, "r");
    if (!state.input_file) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", file_name);
        return EOF;
    }
    yyin = state.input_file;
    return 0;
}

int console_run(void) {
    if (!state.input_file || !state.cfg)
        return EOF;
    int token;
    while ((token = yylex()) != 0);
    return 0;
}

void console_cleanup(void) {
    if (state.input_file) {
        fclose(state.input_file);
        state.input_file = NULL;
    }
    memset(&state, 0, sizeof(state));
}