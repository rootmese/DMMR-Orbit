#ifndef __DMMR_SERVER_H__
#define __DMMR_SERVER_H__

#include <dmmr_circle_buffer.h>
#include <dmmr_parser.h>

struct cfg_daemon_server{
    unsigned char cfg_file[0x400];
    void(*server_handle_token)(struct dmmr_parser*, const char*, const char*);
};

struct cfg_server_server{
    uint64_t scheduler_preemptive_deadline;
};

struct dmmr_server {
    struct circle_buffer* cb;
    struct dmmr_parser* parser;
    struct cfg_daemon_server cfg_daemon;
    
    void (*init)(struct dmmr_server*, const char*);
    void (*run)(struct dmmr_server*);
    void (*shutdown)(struct dmmr_server*);
};

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server*);
void free_dmmr_server(struct dmmr_server*);

#endif // __DMR_SERVER_H__