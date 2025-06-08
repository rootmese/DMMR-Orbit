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