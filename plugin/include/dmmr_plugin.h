#ifndef __DMMR_PLUGIN_H__
#define __DMMR_PLUGIN_H__

#include <defs.h>

struct dmmr_plugin{
	void (*reload)(struct cfg_server_server*);
};

#endif
