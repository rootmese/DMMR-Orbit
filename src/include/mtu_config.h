#ifndef __MTU_CONFIG_H__
#define __MTU_CONFIG_H__

#ifndef MTU_PROFILE_JUMBO
# ifndef MTU_PROFILE_MINIMAL
#   define MTU_PROFILE_1500
# endif
#endif

#if defined(MTU_PROFILE_JUMBO)
#  define DEFAULT_MTU 9000
#elif defined(MTU_PROFILE_MINIMAL)
#  define DEFAULT_MTU 576
#else
#  define DEFAULT_MTU 1500
#endif


#define IP_HEADER_SIZE       20
#define TCP_HEADER_SIZE      40
#define UDP_HEADER_SIZE      8
#define MAX_HEADER_OVERHEAD  80


#define VALUE_BUFFER_SIZE    (DEFAULT_MTU - MAX_HEADER_OVERHEAD)

#define VALUE_OUTPUT_SIZE 6

#define VALUE_OUTPUT_BUFFER_SIZE (VALUE_BUFFER_SIZE * VALUE_OUTPUT_SIZE)


_Static_assert(VALUE_BUFFER_SIZE >= 512, "MTU insuficiente para tráfego útil");

#endif // __MTU_CONFIG_H__
