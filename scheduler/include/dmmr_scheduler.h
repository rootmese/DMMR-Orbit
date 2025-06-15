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
    pthread_t send_thread;
    struct dmmr_socket *sock;
    int (*start)(void);
    void (*stop)(void);
    void (*trigger_send)(struct scheduler_connection*);
    void (*send)(struct node*, unsigned);
    void (*reload)(void);
    int (*insert)(struct session_connection_pool*);
    void(*delete)(struct session_connection_pool*);
};

struct dmmr_scheduler* new_dmmr_scheduler(struct cfg_server_server*);
void destroy_dmmr_scheduler(struct dmmr_scheduler*);

#endif
