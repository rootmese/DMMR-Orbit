#ifndef ____VCPY_H__
#define ____VCPY_H__

#include <stddef.h>

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
