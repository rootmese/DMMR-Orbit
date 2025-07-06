#ifndef __FUNCS_H__
#define __FUNCS_H__

#include <structs.h>

#ifndef __DMMR_SLEEP_H__
#define __DMMR_SLEEP_H__

static inline void nsleep_us(unsigned long us) {
    struct timespec ts;
#ifdef __FreeBSD__
    clock_gettime(CLOCK_MONOTONIC_FAST, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    ts.tv_nsec += us * 1000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
}

#define NSLEEP_US(x) nsleep_us(x)

#endif

#ifndef __CRC_H__
#define __CRC_H__

static uint32_t crc32_table[256];

static inline void generate_crc32_table() {
    uint32_t poly = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ poly : crc >> 1;
        crc32_table[i] = crc;
    }
}

static inline uint32_t crc32_buffer(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

static uint32_t crc32_file(const uint8_t* path) {
    FILE* f = fopen((char*)path, "rb");
    if (!f) return 0;

    uint8_t buf[1024];
    size_t bytes;
    uint32_t crc = 0xFFFFFFFF;

    while ((bytes = fread(buf, 1, sizeof(buf), f)) > 0)
        for (size_t i = 0; i < bytes; i++)
            crc = (crc >> 8) ^ crc32_table[(crc ^ buf[i]) & 0xFF];

    fclose(f);
    return crc ^ 0xFFFFFFFF;
}
#endif

#ifndef ____VLEN_H__
#define ____VLEN_H__

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__LP64__) || defined(_LP64)
  #define LONG_BIT 64
#else
  #define LONG_BIT 32
#endif

#if LONG_BIT == 32
static const unsigned long mask01 = 0x01010101;
static const unsigned long mask80 = 0x80808080;
#elif LONG_BIT == 64
static const unsigned long mask01 = 0x0101010101010101;
static const unsigned long mask80 = 0x8080808080808080;
#endif

#define	LONGPTR_MASK (sizeof(long) - 1)

#define testbyte(x)				\
	do {					\
		if (*(p + x) == '\0')		\
		    return (p - str + x);	\
	} while (0)

static inline size_t __vlen(const char *str){
	const char *p;
	const unsigned long *lp;
	long va, vb;

	lp = (const unsigned long *)((uintptr_t)str & ~LONGPTR_MASK);
	va = (*lp - mask01);
	vb = ((~*lp) & mask80);
	lp++;
	if (va & vb)
		for (p = str; p < (const char *)lp; p++)
			if (*p == 0)
				return (p - str);

	for (; ; lp++) {
		va = (*lp - mask01);
		vb = ((~*lp) & mask80);
		if (va & vb) {
			p = (const char *)(lp);
			testbyte(0);
			testbyte(1);
			testbyte(2);
			testbyte(3);
#if (LONG_BIT >= 64)
			testbyte(4);
			testbyte(5);
			testbyte(6);
			testbyte(7);
#endif
		}
	}

	return 0;
}
#endif


#ifndef ____VCPY_H__
#define ____VCPY_H__

#if INTPTR_MAX == INT64_MAX
    typedef uint64_t word_t;
#elif INTPTR_MAX == INT32_MAX
    typedef uint32_t word_t;
#elif INTPTR_MAX == INT16_MAX
    typedef uint16_t word_t;
#else
    typedef uint8_t word_t;
#endif

static inline void __vcpy(void *__d, void *__o, size_t __s)
{
    if (__d && __o && __s)
    {
        uintptr_t d_addr = (uintptr_t)__d;
        uintptr_t o_addr = (uintptr_t)__o;

        if (!(d_addr % sizeof(word_t)) && (!(o_addr % sizeof(word_t))))
        {
            word_t *dst = (word_t *)__d;
            word_t *src = (word_t *)__o;
            size_t w = __s / sizeof(word_t);
            __s %= sizeof(word_t);
            if (w) {
                do {
                    *dst++ = *src++;
                } while (--w);
            }
            __d = dst;
            __o = src;
        }
        char *c0 = (char *)__d;
        char *c1 = (char *)__o;
        if (__s) {
            do {
                *c0++ = *c1++;
            } while (--__s);
        }
    }
}

#endif // ____VCPY_H__


#ifndef ____VCMP_H__
#define ____VCMP_H__

#if defined(__arm__) || defined(__ARM_ARCH) || \
    defined(__mips__) || defined(__riscv) ||   \
    defined(ARCH_SAFE_MEMCPY)

#include <string.h>

#define __vcpy memcmp

#else
static inline int __vcmp(const void *s1, const void *s2, size_t n)
{
	if (n) {
		register const unsigned char *p1 = s1, *p2 = s2;

		do {
			if (!(*p1++ == *p2++))
				return (*--p1 - *--p2);
		} while (--n);
	}
	return 0;
}

#endif

#endif


#ifndef ____NODE_CMP_H__
#define ____NODE_CMP_H__

static inline int node_cmp(const struct node *a, const struct node *b) {
    if (!a || !b)
        return (a > b) - (a < b);

    if (a->fd != b->fd)
        return (a->fd > b->fd) - (a->fd < b->fd);

    if (a->port != b->port)
        return (a->port > b->port) - (a->port < b->port);

    if (a->family != b->family)
        return (a->family > b->family) - (a->family < b->family);

    if (a->family == AF_INET) {
        const struct sockaddr_in *sa = (const struct sockaddr_in *)&a->ipv4;
        const struct sockaddr_in *sb = (const struct sockaddr_in *)&b->ipv4;
        return __vcmp(&sa->sin_addr, &sb->sin_addr, sizeof(struct in_addr));
    }

    if (a->family == AF_INET6) {
        const struct sockaddr_in6 *sa6 = (const struct sockaddr_in6 *)&a->ipv6;
        const struct sockaddr_in6 *sb6 = (const struct sockaddr_in6 *)&b->ipv6;
        return __vcmp(&sa6->sin6_addr, &sb6->sin6_addr, sizeof(struct in6_addr));
    }

    return 0;
}


#endif

#ifndef ____MSET_H__
#define ____MSET_H__


#if defined(__arm__) || defined(__ARM_ARCH) || \
    defined(__mips__) || defined(__riscv) ||   \
    defined(ARCH_SAFE_MEMSET)

#include <string.h>
#define __mset memset

#else // arquiteturas tipo x86, x86_64: pode usar versão otimizada

#define IS_CONSTANT_BYTE(c) (0 == ((c) & ~0xFF))

static inline void *__mset(void *s, int c, size_t n)
{
    if (!n)
        return s;

    char *xs = s;

    if (IS_CONSTANT_BYTE(c)) {
        if (n >= 16) {
            size_t align = (16 - ((uintptr_t)xs & 15)) & 15;
            if (align > 0 && align <= n) {
                n -= align;
                do {
                    *xs++ = c;
                } while (--align);
            }

            size_t n16 = n / 16;
            if (n16 > 0) {
                uint64_t c16 = (uint64_t)(unsigned char)c;
                c16 |= c16 << 8;
                c16 |= c16 << 16;
                c16 |= c16 << 32;

                uint64_t *x16 = (uint64_t *)xs;
                do {
                    *x16++ = c16;
                    *x16++ = c16;
                } while (--n16);

                xs = (char *)x16;
                n &= 15;
            }
        }

        while (n--)
            *xs++ = c;

    } else {
        while (n--)
            *xs++ = c;
    }

    return s;
}

#endif // fallback ou otimizado

#endif // ____MSET_H__



#ifndef ____BCPY_U__
#define ____BCPY_U__

static inline void __bcpy(const void *src0, void *dst0, size_t length)
{
char *dst = dst0;
const char *src = src0;
size_t t;
unsigned long u;

	if(!length || dst == src) /* nothing to do */
		return;

#define	wsize	sizeof(long)
#define	wmask	(wsize - 1)

	if ((unsigned long)dst < (unsigned long)src) /* Copy forward. */
	{
		u = (unsigned long)src;
		if ((u | (unsigned long)dst) & wmask)
		{
			t = ((u ^ (unsigned long)dst) & wmask || length < wsize) ? (length) : (wsize - (size_t)(u & wmask));

			length -= t;
			do
			    *dst++ = *src++;
			while(--t);
		}

		t = length / wsize;
		if(t)
		{
			do
			{
			      *(long *)(void *)dst = *(const long *)(const void *)src, src += wsize, dst += wsize;
			}
			while(--t);
		}
		t = length & wmask;
		if(t)
		{
			do
			{
				*dst++ = *src++;
			}
			while(--t);
		}
	}
	else /* Copy backwards */
	{
		src += length;
		dst += length;

		u = (unsigned long)src;
		if ((u | (unsigned long)dst) & wmask)
		{
			t = ((u ^ (unsigned long)dst) & wmask || length <= wsize) ? (length) : ((size_t)(u & wmask));
			length -= t;
			do
			{
				*--dst = *--src;
			}
			while(--t);
		}
		t = length / wsize;
		if(t)
		{
			do
			{
				src -= wsize, dst -= wsize, *(long *)(void *)dst = *(const long *)(const void *)src;
			}
			while(--t);
		}
		t = length & wmask;
		if(t)
		{
			do
			{
				*--dst = *--src;
			}
			while(--t);
		}
	}
}

#endif

#ifndef __RB_LOG_ENABLE__
#define __RB_LOG_ENABLE__

static inline int rb_log_enabled(uint8_t __m, rb_log_level __l) {
    return (__m & __l);
}
#endif

#ifndef __RB_LOG_LEVEL_NAME__
#define __RB_LOG_LEVEL_NAME__
static inline const char* rb_log_level_name(rb_log_level __l) {
    switch(__l) {
        case rb_log_level_fatal:
            return "FATAL";
        case rb_log_level_error:
            return "ERROR";
        case rb_log_level_warn:
            return "WARN";
        case rb_log_level_info:
            return "INFO";
        case rb_log_level_debug:
            return "DEBUG";
        default:
            return "???";
    }
}
#endif

#ifndef __SPAW_DETACHED_THREADS_H__
#define __SPAW_DETACHED_THREADS_H__

union protocol_base_cb;


static inline void spawn_detached_thread(pthread_t *t, void *(*fn)(void *), void *arg, int *ret)
{
    if (!(*ret = pthread_create(t, 0, fn, arg)))
        pthread_detach(*t);
    // TODO - colocar logs baseado em errno e strerror
}

#endif

#ifndef __SPAW_DETACHED_WITH_ATTRS_THREADS_H__
#define __SPAW_DETACHED_WITH_ATTRS_THREADS_H__

static inline void spawn_detached_thread_with_attr(pthread_t *t, pthread_attr_t *attr, void *(*fn)(void *), void *arg, int *ret)
{
    *ret = 0;
    if (!(*ret = pthread_create(t, attr, fn, arg)))
        pthread_detach(*t);
    // TODO - colocar logs baseado em errno e strerror
}

#endif

#ifndef __LINK_SESSION_PORTS_H__
#define __LINK_SESSION_PORTS_H__

static inline void link_session_ports(struct session_connection_pool *p, union protocol_base_cb *cb) {
    if (!p || !cb)
        return;

    p->port = 0;
    p->linked_port = 0;

    switch (cb->none.proto) {
        case proto_tcp_t:
            if (cb->tcp.node)
                p->port = cb->tcp.node->port;
            if (cb->tcp.linked && cb->tcp.linked->tcp.node)
                p->linked_port = cb->tcp.linked->tcp.node->port;
            break;

        case proto_udp_t:
            if (cb->udp.node)
                p->port = cb->udp.node->port;
            if (cb->udp.linked && cb->udp.linked->udp.node)
                p->linked_port = cb->udp.linked->udp.node->port;
            break;

        default:
            break;
    }
}

#endif

#ifndef __GET_SESSION_POINTER__
#define __GET_SESSION_POINTER__

static inline union protocol_base_cb *get_session_pointer(union protocol_base_cb *__u){
    if(__u){
        switch(__u->none.proto){
            case proto_none_t:
                return 0;
            case proto_tcp_t:
                return __u->tcp.linked;
                break; /* Stupid break */
            case proto_udp_t:
                return __u->udp.linked;
                break; /* Stupid break */
            default:
                return 0;
                break; /* Stupid break */
        }
    }
}

#endif

#ifndef __GET_SESSION_CFG_H__
#define __GET_SESSION_CFG_H__

static inline  void get_session_cfg(union protocol_base_cb *__u, uint16_t *__p0, proto_arr_t *__p1){
    if(__u){
        switch(__u->none.proto){
            case proto_none_t:
                break;
            case proto_tcp_t:
                *__p0 = __u->tcp.node->port;
                *__p1 = proto_arr_tcp_t;
                break;
            case proto_udp_t:
                *__p0 = __u->udp.node->port;
                *__p1 = proto_arr_udp_t;
                break;
            default:
                break; /* Stupid break */
        }
    }
}

#endif

#ifndef __PARSE_URI_H__
#define __PARSE_URI_H__

// Defina HAVE_STRLCPY se seu sistema já tiver a função
#ifndef HAVE_STRLCPY
// #define HAVE_STRLCPY 1
#endif

#define __UDP_PROTO__         "UDP/"
#define __UDP_PROTO_LEN__    (sizeof(__UDP_PROTO__) - 1)

#define __TCP_PROTO__         "TCP/"
#define __TCP_PROTO_LEN__    (sizeof(__TCP_PROTO__) - 1)

#define __HOST_URI_SEP__     ':'

static inline proto_t parse_protocol_host_port(const unsigned char *input, unsigned char *host_out, size_t host_size, uint16_t *port_out) {
    if (!input || !host_out || host_size == 0 || !port_out)
        return proto_none_t;

    proto_t proto = proto_none_t;

    if (strncmp((char*)input, __UDP_PROTO__, __UDP_PROTO_LEN__) == 0) {
        proto = proto_udp_t;
        input += __UDP_PROTO_LEN__;
    }
    else if (strncmp((char*)input, __TCP_PROTO__, __TCP_PROTO_LEN__) == 0) {
        proto = proto_tcp_t;
        input += __TCP_PROTO_LEN__;
    }
    else
        return proto_none_t;

    const char *sep = strrchr((const char*)input, __HOST_URI_SEP__);
    if (!sep || sep == (const char*)input)
        return proto_none_t;

    size_t host_len = (size_t)(sep - (const char*)input);
    if (host_len >= host_size)
        host_len = host_size - 1;

    __vcpy(host_out, input, host_len);
    host_out[host_len] = '\0';

    *port_out = (uint16_t)atoi(sep + 1);
    return proto;
}

#endif

#ifndef __CUT_QUOTES_H__
#define __CUT_QUOTES_H__

static void unquote_string(char *dest, size_t dest_size, const char *input) {
    if (!input || !dest || dest_size == 0) return;

    size_t len = __vlen(input);
    if (len >= 2 && input[0] == '"' && input[len - 1] == '"') {
        size_t inner_len = len - 2;
        if (inner_len >= dest_size) inner_len = dest_size - 1;
        __vcpy(dest, input + 1, inner_len);
        dest[inner_len] = '\0';
    } else {
        strncpy(dest, input, dest_size - 1);
        dest[dest_size - 1] = '\0';
    }
}


#endif

#ifndef __UPATE_SESSION_COUNTER_H__
#define __UPATE_SESSION_COUNTER_H__

static inline void update_session_counter(union protocol_base_cb *__u){
	        switch(__u->none.proto){
            case proto_udp_t:{
                unsigned s = 0;
                struct udp_node *p = &(__u->udp);
                struct node *n = p->node, *n0 = n + p->node_count;
                for(; n < n0; ++n)
                    s += n->value_size;
                p->node_count = s;
            }
                break;
            case proto_tcp_t:{
                unsigned s = 0;
                struct tcp_node *p = &(__u->tcp);
                struct node *n = p->node, *n0 = n + p->node_count;
                for(; n < n0; ++n)
                    s += n->value_size;
                p->node_count = s;
            }
                break;
            default:
                break; /* Stupid break */
        }
}
#endif

#ifndef __GET_SESSION_NODE_H__
#define __GET_SESSION_NODE_H__

static inline struct node* get_session_node(union protocol_base_cb *__u){
	        switch(__u->none.proto){
            case proto_udp_t:{
                struct udp_node *p = &(__u->udp);
                return p->node;
            }
                break;
            case proto_tcp_t:{
                struct tcp_node *p = &(__u->tcp);
                return p->node;
            }
                break;
            default:
                return 0;
                break; /* Stupid break */
        }
}

#endif

#ifndef __DNS_UTILS_H__
#define __DNS_UTILS_H__

static inline ezp_addr_type dns2ipaddr(const char *in, unsigned char *out) {
    int status;
    char ipstr[INET6_ADDRSTRLEN];
    struct addrinfo hints, *res, *p;

    __mset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(in, 0, &hints, &res);
    if(!(status == 0))
        return EZP_INVALID;
    for(p = res; p; p = p->ai_next) {
        switch(p->ai_family){
            case AF_INET:
                __vcpy(out, (struct sockaddr_in*)p->ai_addr, sizeof(struct sockaddr_in));
                freeaddrinfo(res);
                return (ezp_addr_type)AF_INET;
                break; /* Stupid break :P ~*/
            case AF_INET6:
                __vcpy(out, (struct sockaddr_in6*)p->ai_addr, sizeof(struct sockaddr_in6));
                freeaddrinfo(res);
                return (ezp_addr_type)AF_INET6;
                break; /* Stupid break :P ~*/
            default:
                continue;
        }
    }
    freeaddrinfo(res);
    return EZP_INVALID;
}

#endif

#ifndef __PARSE_URI_H__
#define __PARSE_URI_H__

// Defina HAVE_STRLCPY se seu sistema já tiver a função
#ifndef HAVE_STRLCPY
// #define HAVE_STRLCPY 1
#endif

#define __UDP_PROTO__         "UDP/"
#define __UDP_PROTO_LEN__    (sizeof(__UDP_PROTO__) - 1)

#define __TCP_PROTO__         "TCP/"
#define __TCP_PROTO_LEN__    (sizeof(__TCP_PROTO__) - 1)

#define __HOST_URI_SEP__     ':'

static inline proto_t parse_protocol_host_port(const unsigned char *input, unsigned char *host_out, size_t host_size, uint16_t *port_out) {
    if (!input || !host_out || host_size == 0 || !port_out)
        return proto_none_t;

    proto_t proto = proto_none_t;

    if (strncmp((char*)input, __UDP_PROTO__, __UDP_PROTO_LEN__) == 0) {
        proto = proto_udp_t;
        input += __UDP_PROTO_LEN__;
    }
    else if (strncmp((char*)input, __TCP_PROTO__, __TCP_PROTO_LEN__) == 0) {
        proto = proto_tcp_t;
        input += __TCP_PROTO_LEN__;
    }
    else
        return proto_none_t;

    const char *sep = strrchr((const char*)input, __HOST_URI_SEP__);
    if (!sep || sep == (const char*)input)
        return proto_none_t;

    size_t host_len = (size_t)(sep - (const char*)input);
    if (host_len >= host_size)
        host_len = host_size - 1;

    __vcpy(host_out, input, host_len);
    host_out[host_len] = '\0';

    *port_out = (uint16_t)atoi(sep + 1);
    return proto;
}



#endif

#ifndef __IP_ADDR_H__
#define __IP_ADDR_H__

static inline ezp_addr_type ezp_address(const char *input) {
    struct in_addr ipv4;
    struct in6_addr ipv6;
    if (inet_pton(AF_INET, input, &ipv4) == 1) {
        return EZP_IPV4;
    }
    if (inet_pton(AF_INET6, input, &ipv6) == 1) {
        return EZP_IPV6;
    }
    for (size_t i = 0; i < strlen(input); i++) {
        if (!isalnum(input[i]) && input[i] != '-' && input[i] != '.') {
            return EZP_INVALID;
        }
    }
    return EZP_DNS;
}

#endif




#endif