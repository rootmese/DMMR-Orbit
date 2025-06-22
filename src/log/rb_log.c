// rb_log.c
#include <rb_log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <__mset.h>

static struct cfg_server_server *cfg;
static struct rb_log_buffer log_buf;
static pthread_t log_thread;
static int run = 0;

// TODO o sistema de log é feito para usar syslog ou similar, não deve gerar IO na versão de produção
static void* rb_log_thread_func(void* arg) {
    do {
        pthread_mutex_lock(&log_buf.lock);
        while (log_buf.read_pos == log_buf.write_pos && run)
            pthread_cond_wait(&log_buf.cond, &log_buf.lock);
        while (log_buf.read_pos != log_buf.write_pos) {
            fputs(log_buf.queue[log_buf.read_pos], stderr);
            fputc('\n', stderr);
            fflush(stderr);
            log_buf.read_pos = (log_buf.read_pos + 1) % RB_LOG_LINE_SIZE;
        }
        pthread_mutex_unlock(&log_buf.lock);
    }while (run);
    return 0;
}

void rb_log_init(struct cfg_server_server *server_cfg) {
    cfg = server_cfg;
    __mset(&log_buf, 0, sizeof(log_buf));
    pthread_mutex_init(&log_buf.lock, 0);
    pthread_cond_init(&log_buf.cond, 0);
    pthread_create(&log_thread, 0, rb_log_thread_func, 0);
}

void rb_log_shutdown(void) {
    pthread_mutex_lock(&log_buf.lock);
    run = 0;
    pthread_cond_signal(&log_buf.cond);
    pthread_mutex_unlock(&log_buf.lock);
    pthread_join(log_thread, 0);
    pthread_mutex_destroy(&log_buf.lock);
    pthread_cond_destroy(&log_buf.cond);
}

void rb_log_push(rb_log_level level, const char *msg, const char *func, int line) {
    if(rb_log_enabled(cfg->log_level, level)){
        pthread_mutex_lock(&log_buf.lock);
        char *buf = log_buf.queue[log_buf.write_pos];
        char *p = buf;
        const char *end = buf + RB_LOG_LINE_SIZE - 1;

        while (*func && p < end)
            *p++ = *func++;
        if (p < end) *p++ = ' ';
        if (p < end) *p++ = '(';
        char num[16];
        char *num_ptr = num + sizeof(num) - 1;
        *num_ptr = 0;
        int abs_line = line < 0 ? -line : line;
        do {
            *--num_ptr = 0 + (abs_line % 10);
            abs_line /= 10;
        } while (abs_line > 0);
        if (line < 0) *--num_ptr = '-';
        for (char *n = num_ptr; *n && p < end; ) *p++ = *n++;
        if (p < end) *p++ = ')';
        if (p < end) *p++ = ' ';
        while (*msg && p < end) *p++ = *msg++;
        *p = 0;
        log_buf.write_pos = (log_buf.write_pos + 1) % RB_LOG_LINE_SIZE;

        pthread_cond_signal(&log_buf.cond);
        pthread_mutex_unlock(&log_buf.lock);
        
    }
}