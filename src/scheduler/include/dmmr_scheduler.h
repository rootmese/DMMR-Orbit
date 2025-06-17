#ifndef __DMMR_SCHEDULER_H__
#define __DMMR_SCHEDULER_H__

#include <defs.h>
#include <dmmr_socket.h>

struct dmmr_scheduler {
    uint64_t deadline;
    void *session_connection_queue;
    pthread_t reorder_thread;
    pthread_t send_thread;
    struct dmmr_socket *sock;
    int (*start)(void);
    void (*stop)(void);
    void (*trigger_send)(struct scheduler_connection*);
    void (*send)(struct scheduler_connection*, struct node*, unsigned);
    void (*reload)(void);
    int (*insert)(struct session_connection_pool*);
    void(*delete)(struct session_connection_pool*);
};

struct dmmr_scheduler* new_dmmr_scheduler(struct dmmr_socket*, struct cfg_server_server*);
void destroy_dmmr_scheduler(struct dmmr_socket*, struct dmmr_scheduler*);

#endif
