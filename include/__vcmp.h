#ifndef ____VCMP_H__
#define ____VCMP_H__

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