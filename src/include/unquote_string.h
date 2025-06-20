#ifndef __CUT_QUOTES_H__
#define __CUT_QUOTES_H__

#include <stdio.h>
#include <string.h>

static void unquote_string(char *dest, size_t dest_size, const char *input) {
    if (!input || !dest || dest_size == 0) return;

    size_t len = strlen(input);
    if (len >= 2 && input[0] == '"' && input[len - 1] == '"') {
        size_t inner_len = len - 2;
        if (inner_len >= dest_size) inner_len = dest_size - 1;
        memcpy(dest, input + 1, inner_len);
        dest[inner_len] = '\0';
    } else {
        strncpy(dest, input, dest_size - 1);
        dest[dest_size - 1] = '\0';
    }
}


#endif