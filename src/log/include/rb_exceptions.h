#ifndef __RB_EXCEPTIONS_H__
#define __RB_EXCEPTIONS_H__

#include <errno.h>

typedef enum {
    RB_EXC_OK = 0,
    RB_EXC_ALLOC_FAIL = 1,
    RB_EXC_NULL_CONFIG,
    RB_EXC_SERVER_INIT,
    RB_EXC_FORK_FAIL,
    RB_EXC_POSIX_ERROR,       // Usado pra encapsular errno
    RB_EXC_UNKNOWN,
    RB_EXC_MAX
} rb_exception_code;

static const char* const rb_exception_messages[] = {
    [RB_EXC_OK]            = "Sem erro",
    [RB_EXC_ALLOC_FAIL]    = "Falha de alocação (calloc)",
    [RB_EXC_NULL_CONFIG]   = "Ponteiro de configuração nulo",
    [RB_EXC_SERVER_INIT]   = "Erro ao criar servidor",
    [RB_EXC_FORK_FAIL]     = "Erro no fork()",
    [RB_EXC_POSIX_ERROR]   = "Erro POSIX (use strerror)",
    [RB_EXC_UNKNOWN]       = "Exceção desconhecida"
};

static inline const char* rb_exc_message(rb_exception_code code) {
    return (code < RB_EXC_MAX ? rb_exception_messages[code] : "Código inválido");
}

#endif
