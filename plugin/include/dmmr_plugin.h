#ifndef __DMMR_PLUGIN_H__
#define __DMMR_PLUGIN_H__

#include <defs.h>

struct dmmr_plugin{
	void (*reload)(struct cfg_server_server*);
	void (*load)(struct dmmr_session_connection_manager*);
};

struct dmmr_plugin *new_dmmr_plugin(struct dmmr_session_connection_manager*, struct cfg_server_server*);

#endif
