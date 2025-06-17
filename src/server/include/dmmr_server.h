#ifndef __DMMR_SERVER_H__
#define __DMMR_SERVER_H__

#include <defs.h>
#include <dmmr_session_connection_manager.h>

struct dmmr_server {
    struct cfg_daemon_server *cfg_daemon;
    struct cfg_server_server *cfg;
    
    int (*run)(void);
    void (*stop)(void);
};

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server*);
void free_dmmr_server(struct dmmr_server*);


#endif // __DMR_SERVER_H__