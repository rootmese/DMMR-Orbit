#include <stdio.h>

#include <__vcpy.h>
#include <__mset.h>

#include <node_recv_manager.h>

struct node *recicle_node(struct node_recv_manager *input, uint8_t pos){
	if(input && pos && pos < 0x100){
		struct node_buffer *p = input->buffer + pos;
		p->node_buffer_status = 0;
	}
	return 0;
}

struct node *get_free_node(struct node_recv_manager *input, uint8_t *pos){
	if(input && pos && *pos < 0x100){
		struct node_buffer *p = input->buffer + *pos;
		struct node_buffer *p1 = input->buffer + 0x100;
		do {
			if (!p->node_buffer_status){ //se o node está liberado não tem ninguém usando ele, pode gravar ser lock
				p->node_buffer_status = !0;
				*pos = (p1 - p) & 0xFF;
				return &(p->node);
			}
		}while (++p < p1);
	}
	return 0;
}

struct node *get_buzy_node(struct node_recv_manager *input, uint8_t *pos){
	if(input && pos && *pos < 0x100){
		struct node_buffer *p = input->buffer + *pos;
		struct node_buffer *p1 = input->buffer + 0x100;
		do {
			if(p->node_buffer_status){
				p->node_buffer_status = 0;
				*pos = (p1 - p) & 0xFF;
				return &(p->node);
			}
		}while (++p < p1);
	}
	return 0;
}

unsigned count_node(struct node_recv_manager *input){
	unsigned ret = 0;
	if(input){
		struct node_buffer *p = input->buffer;
		struct node_buffer *p1 = input->buffer + 0x100;
		do {
			if(p->node_buffer_status)
				++ret;
		}while (++p < p1);
	}
	return ret;
}

struct node *copy_buffer(struct node_recv_manager *instance, struct node *output, uint8_t *pos){
	if (instance && output && pos && *pos < 0x100){
		struct node_buffer *p = instance->buffer + *pos;
		struct node_buffer *p1 = instance->buffer + 0x100;
		do{
			if(p->node_buffer_status){
				__vcpy(output, &(p->node), sizeof(struct node));
				break;
			}
		}while(++p < p1);
		*pos = (p - instance->buffer + 1) % 0x100;
		return get_buzy_node(instance, pos);
	}
	*pos = 0;
	return 0;
}

int init_node_recv_manager(struct node_recv_manager *input){
	if(input){
		__mset(input, 0, sizeof(struct node_recv_manager));
		pthread_mutex_init(&(input->mutex), 0);
		return 0;
	}
	else
		return EOF;
}

void stop_node_recv_manager(struct node_recv_manager *input){
	if(input){
		pthread_mutex_destroy(&(input->mutex));
		__mset(input, 0, sizeof(struct node_recv_manager));
	}
}