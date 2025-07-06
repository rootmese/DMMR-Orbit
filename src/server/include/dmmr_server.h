#ifndef __DMMR_SERVER_H__
#define __DMMR_SERVER_H__

#include <funcs.h>
#include <dmmr_session_connection_manager.h>

struct dmmr_server {
    
    int (*run)(void);
    void (*stop)(void);
    int (*start)(void);
    int (*reload)(void);
};

struct dmmr_server* new_dmmr_server(struct cfg_daemon_server*);
void free_dmmr_server(struct dmmr_server*);


#endif // __DMR_SERVER_H__