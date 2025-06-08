#ifndef __DMMR_SESSION_CONNECTION_MANAGER_H__
#define __DMMR_SESSION_CONNECTION_MANAGER_H__

struct dmmr_session_connection_manager{
	void (*reload)(struct cfg_server_server*);
	void (*enqueue)(struct node*, unsigned); //callback to circle_buffer insert data
};

#endif