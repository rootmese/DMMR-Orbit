#ifndef __DMMR_SOCKET_H__
#define __DMMR_SOCKET_H__

struct dmmr_socket{
	void (*acception_cb)(struct node*); //populate circle buffer
	void (*dispatcher_cb)(union protocol_base_cb*); // warm for consume create a new connection to send data
	int (*start_acception)(proto_t, ezp_addr_type, uint16_t, const char*);
	int (*stop_acception)(proto_t, ezp_addr_type, uint16_t, const char*);
	int (*start_dispatcher)(proto_t, ezp_addr_type, uint16_t, const char*);
	void (*stop_dispatcher)(proto_t, ezp_addr_type, uint16_t, const char*);
	int (*dispatcher)(union protocol_base_cb*);
	void (*reload)(struct dmmr_scheduler*);
};

struct dmmr_socket *new_dmmr_socket(struct cfg_server_server*, void (*acception_cb)(struct node*), void (*dispatcher_cb)(union protocol_base_cb*));


#endif
