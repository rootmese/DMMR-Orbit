#ifndef ____VCPY_H__
#define ____VCPY_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
