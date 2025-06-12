#ifndef __DMMR_SCHEDULER_H__
#define __DMMR_SCHEDULER_H__

#include <defs.h>

#include <dmmr_session_connection_manager.h>


struct scheduler_connection {
    uint64_t last_active_time_us;
    uint64_t realtime_deadline_us;
    uint64_t deadline_us;
    struct session_connection_pool *session_ptr; // <-- ponteiro direto para a session_connection real
};


struct dmmr_scheduler {
    uint64_t deadline;
    void *session_connection_queue;
    pthread_t reorder_thread;
    struct dmmr_socket *sock;
    int (*start)(struct dmmr_scheduler*);
    void (*stop)(struct dmmr_scheduler*);
    void (*trigger_send)(struct dmmr_scheduler*, struct scheduler_connection*);
    void (*reload)(struct dmmr_scheduler*);
    int (*insert)(struct dmmr_scheduler*, struct session_connection_pool*);
    void(*delete)(struct session_connection_pool*);
};

struct dmmr_scheduler* new_dmmr_scheduler(struct cfg_server_server*);
void destroy_dmmr_scheduler(struct dmmr_scheduler*);

#endif
