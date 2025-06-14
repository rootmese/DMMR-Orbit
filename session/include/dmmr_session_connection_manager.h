#ifndef __DMMR_SESSION_CONNECTION_MANAGER_H__
#define __DMMR_SESSION_CONNECTION_MANAGER_H__

#include <dmmr_scheduler.h>
#include <dmmr_circle_buffer.h>



struct dmmr_session_connection_manager{
	void (*reload)(struct cfg_server_server*);
	int (*connect)(const unsigned char*);
	int (*accept)(const unsigned char*);
	void (*close)(const unsigned char*);
	int (*send)(const unsigned char*, unsigned);
	int (*receive)(const unsigned char*, unsigned*);
	void (*trunk)(const unsigned char*, const char*);
};

struct dmmr_session_connection_manager* new_session_connection_manager
(
	struct circle_buffer*,
	struct dmmr_scheduler*,
	struct dmmr_socket*,
	struct cfg_server_server*
);

#endif