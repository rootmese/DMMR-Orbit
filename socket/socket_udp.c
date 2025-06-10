#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/queue.h>

struct udp_receiver_data {
    struct sockaddr_in client_addr;
    socklen_t addr_len;
};

struct udp_node {
    struct node node;
    uint8_t run;
    pthread_t receiver_thread;
    void (*on_receive_cb)(const struct node*);
} udp_node[0x10000];

static struct node *node_pool = 0;
static unsigned node_pool_size = 0;
static unsigned node_pool_count = 0;

static struct node *get_node(void){
    if(node_pool_count >= node_pool_size)
    {
        node_pool_size *= 2;
        node_pool = (struct node*)realloc(node_pool, node_pool_size * sizeof(struct node));
    }
    return node_pool + node_pool_count++;
}

static void* receiver_thread(void* arg) {
    struct udp_node *node = (struct udp_node*)arg;
    struct timespec ts_monotonic;
    struct timespec ts_realtime;
   do {
        struct udp_receiver_data client_data = {
            .addr_len = sizeof(client_data.client_addr)
        };
        struct node *n = get_node();
        n->fd = node->node.fd;
        n->value_size = recvfrom(
            node->node.fd,
            n->value,
            sizeof(n->value),
            0,
            (struct sockaddr*)&client_data.client_addr,
            &client_data.addr_len
        );
        if(n->value_size <= 0)
            if(node->run && n->value_size < 0) 
                continue;
        n->proto = proto_t.proto_udp_t;
        n->port = ntohs(client_data.client_addr.sin_port);
        n->node.sin_addr = client_addr.sin_addr.s_addr;
        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic);
        n->arrival = (uint64_t)ts_monotonic.tv_sec * 1000000000ULL + ts_monotonic.tv_nsec;
        clock_gettime(CLOCK_REALTIME, &ts_realtime);
        n->deadline = (uint64_t)ts_realtime.tv_sec * 1000000000ULL + ts_realtime.tv_nsec;
        if(node->on_receive_cb) {
            node->on_receive_cb(n);
        }
    }while(node->run);
    return 0;
}

int udp_send_node(struct node *node, unsigned node_size) {
    if(!node)
        return EOF;
    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(node->port),
        .sin_addr = { .s_addr = node->sin_addr }
    };
	struct node *p = node, *p1 = node + node_size;
	do{
		(void)sendto(
			p->fd,
			p->value,
			p->value_size,
			0,
			(struct sockaddr*)&dest_addr,
			sizeof(dest_addr)
		);
	}while(++p < p1);
    return 0;
}


int start_udp_service(uint16_t port, void (*on_receive_cb)(const struct node*)) 
{
    if(!pool)
        (void)start_udp_socket();
    struct udp_node* service = &udp_node[port & 0xFFFf];
    service->on_receive_cb = on_receive_cb;
    service->run = !0;
    if((service->node.fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return EOF;
    }
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };
    if(bind(service->node.fd, (struct sockaddr*)&server_addr, sizeof(server_addr))){
        perror("bind failed");
        close(service->node.fd);
        return EOF;
    }
    if(pthread_create(&service->receiver_thread, 0, receiver_thread, service)) {
        perror("receiver thread creation failed");
        close(service->node.fd);
        return EOF;
    }
    return 0;
}

int start_udp_socket(void){
   node_pool = (struct node*)calloc(0x400, sizeof(struct node));
   if(!node_pool)
       return EOF;
    node_pool_size = 0x400;
    memset(udp_node, 0, sizeof(udp_node));
    return 0;
}

void stop_udp_socket(void){
   int i = 0;
   do {
        udp_node[i].run = 0;
   } while(++i < 0x10000);
   sleep(1);
   if(node_pool)
        free(node_pool);
   return 0;
}
