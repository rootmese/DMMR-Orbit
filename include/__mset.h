#ifndef ____MSET_H__
#define ____MSET_H__

#include <stddef.h>
#include <stdint.h>

// Detecta arquiteturas que exigem alinhamento estrito
#if defined(__arm__) || defined(__ARM_ARCH) || \
    defined(__mips__) || defined(__riscv) ||   \
    defined(ARCH_SAFE_MEMSET)

#include <string.h> // para fallback memset

#define __mset memset

#else

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
                uint64_t c16 = (uint64_t)c;
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
        if (n > 0) {
            do {
                *xs++ = c;
            } while (--n);
        }
    } else {
        do {
            *xs++ = c;
        } while (--n);
    }
    return s;
}

#endif // arquitetura

#endif // ____MSET_H__
