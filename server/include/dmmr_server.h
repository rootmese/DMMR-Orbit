#ifndef __DMMR_SERVER_H__
#define __DMMR_SERVER_H__

#include <defs.h>

struct cfg_daemon_server {
    uint8_t  cfg_file[0x400];
};

struct cfg_server_server{
    uint64_t scheduler_preemptive_deadline;
    uint16_t sleep_time;
    uint16_t session_size;
    uint32_t circle_buffer_size;
    uint16_t max_ports;
    uint16_t __filler1;
    uint64_t real_time_dead_line;
    uint64_t real_time_user_defined;
    uint8_t trunk_accept_uri[0x40];
    uint8_t trunk_dispatch_uri[0x40];
};

struct dmmr_server {
    struct cfg_daemon_server cfg_daemon;
    struct cfg_server_server cfg;
    
    void (*run)(void);
    void (*stop)(void);
};

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server*);
void free_dmmr_server(struct dmmr_server*);

#endif // __DMR_SERVER_H__