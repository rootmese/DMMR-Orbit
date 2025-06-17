#include <stdio.h>
#include <setjmp.h>

typedef struct {
    jmp_buf env;
    int exception_code;
} try_catch_t;

#define TRY(ctx)            if (((ctx).exception_code = setjmp((ctx).env)) == 0)
#define CATCH(ctx)          else
#define THROW(ctx, code)    longjmp((ctx).env, (code))

#define TRY_LOG(ctx)  printf("[TRY] Linha %d\n", __LINE__); TRY(ctx)
#define CATCH_LOG(ctx) printf("[CATCH] Linha %d - erro: %d\n", __LINE__, ctx.exception_code); CATCH(ctx)

/*
// Uma função que pode "lançar" erro
void pode_falhar(try_catch_t *ctx) {
    printf("Executando pode_falhar()\n");
    // Simulando erro
    THROW(*ctx, 42);
}

int main(void) {
    try_catch_t ctx;

    TRY(ctx) {
        printf("Início do bloco TRY\n");
        pode_falhar(&ctx);
        printf("Fim do bloco TRY (isso não será executado se THROW for chamado)\n");
    }
    CATCH(ctx) {
        printf("Exceção capturada! Código = %d\n", ctx.exception_code);
    }

    printf("Continuação do programa\n");
    return 0;
}
*/
