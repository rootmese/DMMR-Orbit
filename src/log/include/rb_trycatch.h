#ifndef __RB_TRYCATCH_H__
#define __RB_TRYCATCH_H__

#include <setjmp.h>
#include <rb_log.h>
#include <rb_exceptions.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#define RB_THROW_ERRNO(ctx, errnum) \
    do { \
        char errbuf[128]; \
        const char *msg = 0; \
        if (strerror_r(errnum, errbuf, sizeof(errbuf)) == 0) \
            msg = errbuf; \
        else \
            msg = "Erro POSIX desconhecido"; \
        rb_log_push(LOG_ERR, msg, __func__, __LINE__); \
        (ctx).exception_code = RB_EXC_POSIX_ERROR; \
        longjmp((ctx).env, RB_EXC_POSIX_ERROR); \
    } while(0)


typedef struct {
    jmp_buf env;
    int exception_code;
} try_catch_t;

#define RB_TRY(ctx) \
    if (((ctx).exception_code = setjmp((ctx).env)) == 0)

#define RB_THROW(ctx, code) \
    do { \
        (ctx).exception_code = code; \
        rb_log_push(LOG_ERR, rb_exc_message(code), __func__, __LINE__); \
        longjmp((ctx).env, code); \
    } while(0)

#define RB_CATCH(ctx) \
    else

#endif
