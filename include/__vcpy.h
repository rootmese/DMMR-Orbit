#ifndef ____VCPY_H__
#define ____VCPY_H__

static inline void __vcpy(void *__d, void *__o, size_t __s)
{
	if(__d && __o && __s)
	{
		size_t s = __s;
		char *c0, *c1;
		unsigned *u0 = (unsigned*)__d, *u1 = (unsigned*)__o;

		if(s > sizeof(unsigned))
		{
			do
			{
				*u0 = *u1;
				++u0, ++u1;
				s -= sizeof(unsigned);
			}
			while(s > sizeof(unsigned));
		}
		if(s)
		{
			c0 = (char*)u0, c1 = (char*)u1;
			while (s--)
				*c0++ = *c1++;
		}
	}
}

#endif
