#ifndef __DMMR_SCHEDULER_H__
#define __DMMR_SCHEDULER_H__

#include <stdint.h>
#include <pthread.h>


struct scheduler_connection {
    uint64_t last_active_time_us;
    uint64_t realtime_deadline_us;
    uint64_t deadline_us;
    void *session_ptr; // <-- ponteiro direto para a session_connection real
};


struct dmmr_scheduler {
    uint64_t deadline;
    void *session_connection_queue;
    pthread_t reorder_thread;
    void (*reload)(struct cfg_server_server*);
};

struct dmmr_scheduler* new_dmmr_scheduler(struct cfg_server_server*);
void destroy_dmmr_scheduler(struct dmmr_scheduler*);

#endif
