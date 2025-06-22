#include <rb_log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <__mset.h>
#include <__vlen.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

static int syslog_fd = -1;
static struct cfg_server_server *cfg;
static struct rb_log_buffer log_buf;
static pthread_t log_thread;
static int run = 0;

static void* rb_log_thread_func(void* arg) {
    struct sockaddr_un syslog_addr;
    __mset(&syslog_addr, 0, sizeof(syslog_addr));
    syslog_addr.sun_family = AF_UNIX;
    strncpy(syslog_addr.sun_path, "/dev/log", sizeof(syslog_addr.sun_path) - 1);

    syslog_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (syslog_fd < 0) {
        perror("socket(/dev/log)");
        return 0;
    }

    if (connect(syslog_fd, (struct sockaddr*)&syslog_addr, sizeof(syslog_addr)) < 0) {
        perror("connect(/dev/log)");
        close(syslog_fd);
        syslog_fd = EOF;
        return 0;
    }

    do {
        pthread_mutex_lock(&log_buf.lock);
        while (log_buf.read_pos == log_buf.write_pos && run)
            pthread_cond_wait(&log_buf.cond, &log_buf.lock);

        // Processamento em batch com vetor de I/O
        while (log_buf.read_pos != log_buf.write_pos) {
            struct iovec iov[16]; // Vetor para até 16 mensagens
            int batch_count = 0;
            int initial_read = log_buf.read_pos;

            // Preenche o batch enquanto houver mensagens
            while (batch_count < 16 && log_buf.read_pos != log_buf.write_pos) {
                iov[batch_count].iov_base = log_buf.queue[log_buf.read_pos];
                iov[batch_count].iov_len = __vlen(log_buf.queue[log_buf.read_pos]);
                
                log_buf.read_pos = (log_buf.read_pos + 1) % RB_LOG_BUFFER_SIZE;
                batch_count++;
            }

            // Envio vetorizado se houver mensagens no batch
            if (syslog_fd >= 0 && batch_count > 0) {
                writev(syslog_fd, iov, batch_count);
            }

            // Fallback se o socket estiver inativo
            else if (syslog_fd < 0 && batch_count > 0) {
                for (int i = 0; i < batch_count; i++) {
                    fputs((char*)iov[i].iov_base, stderr);
                    fputc('\n', stderr);
                }
            }
        }

        pthread_mutex_unlock(&log_buf.lock);
    } while (run);

    if (syslog_fd >= 0){
        close(syslog_fd);
        syslog_fd = EOF;
    }
    return 0;
}

void rb_log_init(struct cfg_server_server *server_cfg) {
    cfg = server_cfg;
    __mset(&log_buf, 0, sizeof(log_buf));
    openlog("dmmr_orbit", LOG_PID | LOG_CONS, LOG_DAEMON);
    pthread_mutex_init(&log_buf.lock, 0);
    pthread_cond_init(&log_buf.cond, 0);
    run = 1;
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
    if (rb_log_enabled(cfg->log_level, level)) {
        pthread_mutex_lock(&log_buf.lock);

        char *buf = log_buf.queue[log_buf.write_pos];
        char *p = buf;
        const char *end = buf + RB_LOG_LINE_SIZE - 1;
        while (*func && p < end) *p++ = *func++;
        if (p < end) *p++ = ' ';
        if (p < end) *p++ = '(';
        char num[16];
        char *num_ptr = num + sizeof(num) - 1;
        *num_ptr = 0;
        int abs_line = line < 0 ? -line : line;
        do {
            *--num_ptr = '0' + (abs_line % 10);
            abs_line /= 10;
        } while (abs_line > 0);
        if (line < 0) *--num_ptr = '-';
        while (*num_ptr && p < end) *p++ = *num_ptr++;
        if (p < end) *p++ = ')';
        if (p < end) *p++ = ' ';
        while (*msg && p < end)
            *p++ = *msg++;
        *p = 0;
        log_buf.write_pos = (log_buf.write_pos + 1) % RB_LOG_BUFFER_SIZE;
        pthread_cond_signal(&log_buf.cond);
        pthread_mutex_unlock(&log_buf.lock);
    }
}
