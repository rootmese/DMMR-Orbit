#ifndef __STRLCPY_H__
#define __STRLCPY_H__

#include <stddef.h>

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif

#endif