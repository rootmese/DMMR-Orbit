#ifndef __DMMR_DAEMON_H__
#define __DMMR_DAEMON_H__

#include <defs.h>
#include <dmmr_server.h>

struct dmmr_daemon {
    int (*start)(void);
    int (*stop)(void);
    int (*run)(void);
    int (*reload)(void);
};

struct dmmr_daemon* new_dmmr_daemon(struct cfg_daemon_server* config);
void free_dmmr_daemon(void);

#endif