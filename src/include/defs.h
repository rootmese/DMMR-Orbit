#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include <mtu_config.h>

#define LOG() printf("[%s:%d]\n", __FUNCTION__, __LINE__)

#define LOG_ERRNO(msg) \
    fprintf(stderr, "[%s:%d] %s: (%d) %s\n", __FUNCTION__, __LINE__, msg, errno, strerror(errno))



#endif