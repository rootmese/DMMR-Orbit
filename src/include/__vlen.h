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
