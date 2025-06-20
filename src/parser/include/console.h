#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <defs.h>

struct cfg_server_server;

// Interface do console
int console_setup(const char *file_name, struct cfg_server_server *cfg);
int console_run(void);
void console_cleanup(void);

#endif