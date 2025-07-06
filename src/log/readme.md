# ?? Sistema de Logs e Exceções - `rb_log` e `rb_trycatch`

Este documento descreve como utilizar o sistema de logs assíncrono (`rb_log`) e o sistema de tratamento de exceções tipo `try/catch` em C (`rb_trycatch`) no projeto.

---

## ?? Módulo: `rb_log`

### ?? Inicialização

Antes de enviar qualquer log, inicialize o sistema:

```c
rb_log_init(cfg);
cfg deve apontar para uma struct cfg_server_server com o campo .log_level definido.

?? Níveis de Log (enum rb_log_level)
c
Copiar
Editar
typedef enum {
    LOG_FATAL = 0,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} rb_log_level;
Esses níveis são utilizados para filtrar os logs enviados de acordo com a configuração ativa.

?? Enviando logs
c
Copiar
Editar
rb_log_push(NÍVEL, "mensagem", __func__, __LINE__);
Exemplo:
c
Copiar
Editar
rb_log_push(LOG_INFO, "Servidor iniciado", __func__, __LINE__);
rb_log_push(LOG_ERROR, "Falha de leitura de socket", __func__, __LINE__);
?? Encerramento do sistema de logs
Libera recursos e encerra a thread de log:

c
Copiar
Editar
rb_log_shutdown();
?? Fallback de log
Se rb_log_init() não for chamado (ou falhar ao conectar com /dev/log), os logs são redirecionados para stderr, sem travar o sistema.

?? Observação
Se cfg == NULL, todos os níveis de log são aceitos automaticamente (modo inicialização segura).

?? Módulo: rb_trycatch
Sistema inspirado em setjmp/longjmp para simular try/catch com códigos de erro e logging.

?? Exemplo de uso
c
Copiar
Editar
try_catch_t ctx;

RB_TRY(ctx) {
    daemonize(&ctx);  // Pode lançar erro
    srv = new_dmmr_server(cfg);
    if (!srv)
        RB_THROW(ctx, RB_EXC_SERVER_INIT);
    rb_log_push(LOG_INFO, "Servidor inicializado", __func__, __LINE__);
}
RB_CATCH(ctx) {
    rb_log_push(LOG_FATAL, rb_exc_message(ctx.exception_code), __func__, __LINE__);
}
?? Lançando exceções
c
Copiar
Editar
RB_THROW(ctx, CÓDIGO_DE_ERRO);
Para erros baseados em errno:

c
Copiar
Editar
RB_THROW_ERRNO(ctx, errno);
?? Mensagens de exceção
Para obter o texto da exceção:

c
Copiar
Editar
rb_exc_message(ctx.exception_code);
?? Considerações em Threads
?? Evite lançar RB_THROW() entre pthread_create() e pthread_join().

Use o try/catch somente dentro de uma mesma thread ou para inicializações síncronas.

? Exemplo completo (extraído de dmmr_daemon.c)
c
Copiar
Editar
static int daemon_start(void) {
    if (!this)
        return EOF;
    try_catch_t ctx;
    RB_TRY(ctx) {
        daemonize(&ctx);
        srv = new_dmmr_server(cfg);
        if (!srv)
            RB_THROW(ctx, RB_EXC_SERVER_INIT);
        rb_log_push(LOG_INFO, "DMMR orbiting.", __func__, __LINE__);
        return 0;
    }
    RB_CATCH(ctx) {
        fprintf(stderr, "[FATAL] new_dmmr_daemon fail: %s\n", rb_exc_message(ctx.exception_code));
        return 0;
    }
}
????? Boas práticas
Use RB_TRY em singletons ou fases de boot.

Sempre registre logs relevantes com nível adequado.

Use stderr como fallback seguro.

Nunca deixe try/catch silencioso sem rb_log_push.

