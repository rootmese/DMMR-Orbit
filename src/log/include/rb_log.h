#ifndef __RB_LOG_H__
#define __RB_LOG_H__

#include <pthread.h>
#include <stdint.h>

#include <funcs.h>

#define RB_LOG_LINE_SIZE   512
#define RB_LOG_BUFFER_SIZE 256

struct rb_log_buffer {
    char queue[RB_LOG_BUFFER_SIZE][RB_LOG_LINE_SIZE];
    uint16_t write_pos;
    uint16_t read_pos;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

void rb_log_init(struct cfg_server_server*);
void rb_log_shutdown(void);
void rb_log_push(rb_log_level, const char*, const char*, int);
void rb_log_flush(void);

#endif