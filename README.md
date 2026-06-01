# DMMR Orbit

DMMR Orbit é um servidor/daemon de rede em C para gerenciamento de sessões e roteamento de tráfego, com plugin de tronco, escalonamento e suporte a operações em foreground ou daemon. O projeto também atua como emulador de tempo real, usando callbacks de I/O para simular comportamento de runtime em tempo real.

## Visão geral

O projeto implementa um servidor chamado `dmmr_orbit` que:
- lê uma configuração de arquivo
- inicia um servidor DMMR customizado de rede
- suporta execução em foreground
- suporta execução como daemon
- usa um parser Flex para configuração
- mantém logs via componente de ring buffer
- emula comportamento de tempo real usando callbacks de I/O e agendamento de sessões

## Estrutura do repositório

- `src/` - código-fonte principal
  - `main.c` - ponto de entrada do programa
  - `daemon/` - controle de execução como daemon
  - `server/` - lógica principal do servidor
  - `parser/` - análise e recarga de configuração
  - `socket/` - abstração de sockets TCP/UDP
  - `session/` - gerência de conexões de sessão
  - `scheduler/` - lógica de escalonamento de runtime
  - `plugin/` - plugin de tronco e integração de sessão
  - `circle_buffer/` - buffer circular de comunicação
  - `log/` - registro e tratamento de logs
  - `utils/` - utilitários auxiliares
- `etc/` - arquivos de configuração
- `makefile` - build system

## Requisitos

- GCC com suporte a GNU11
- `flex`
- sistema POSIX (Linux/Unix)
- biblioteca `librt`

## Build

No diretório raiz do projeto:

```bash
make debug
```

Para build de release:

```bash
make release
```

O binário gerado ficará em `bin/dmmr_orbit`.

## Uso

O binário aceita os parâmetros:

- `-f <arquivo.conf>` - caminho para o arquivo de configuração
- `-d 0|1` - 0 = executar em foreground, 1 = executar como daemon

### Executar em foreground

```bash
./bin/dmmr_orbit -f etc/dmmr_orbit.conf -d 0
```

### Executar como daemon

```bash
./bin/dmmr_orbit -f etc/dmmr_orbit.conf -d 1
```

## Configuração

O arquivo padrão de exemplo está em `etc/dmmr_orbit.conf`. Ele inclui parâmetros como:

- `SCHEDULER_PREEMPTIVE_DEADLINE`
- `SLEEP_TIME`
- `SESSION_SIZE`
- `CIRCLE_BUFFER_SIZE`
- `MAX_PORTS`
- `REAL_TIME_DEAD_LINE`
- `REAL_TIME_USER_DEFINED`
- `TRUNK_ACCEPT_URI`
- `TRUNK_DISPATCH_URI`
- `RB_LOG_LEVEL`

O arquivo de configuração define os valores usados pelo servidor, pelo parser e pelo scheduler.

## Plugins e emulação em tempo real

O DMMR Orbit usa uma interface de plugin simples baseada em callbacks, projetada para comportamento de emulação de tempo real.

A interface principal está em `src/plugin/include/dmmr_plugin.h` e define:

- `struct dmmr_plugin` com:
  - `void (*load)(void);`
  - `void (*reload)(void);`
- `struct dmmr_plugin *new_dmmr_plugin(struct dmmr_session_connection_manager*, struct cfg_server_server*);`

No servidor, o plugin é instanciado e carregado em `src/server/dmmr_server.c`:

```c
plugin = new_dmmr_plugin(session_manager, &cfg);
plugin->load();
```

A implementação atual em `src/plugin/dmmr_trunk_plugin.c` faz a integração com o gerenciador de sessões e registra callbacks de rede para aceitação, conexão, despacho e fechamento de sessões.

Como se trata de um emulador de tempo real, as operações do plugin não são chamadas de forma síncrona tradicional: o plugin usa callbacks para reagir a eventos de I/O e manter a lógica de sessão dentro do loop de runtime.

Isso significa:

- o servidor cria e inicializa o plugin;
- o plugin registra handlers de eventos de rede;
- o plugin administra sessões em tempo real por callbacks de `accept`, `connect`, `dispatch` e `close`.

Essa arquitetura garante que o DMMR Orbit possa comportar-se como um emulador de tempo real, reagindo a eventos de rede conforme eles ocorrem.

O scheduler interno também suporta pré-empção por deadline, com um limite configurável (`SCHEDULER_PREEMPTIVE_DEADLINE`). A lógica de envio é delegada a um thread de scheduler que dispara o despacho quando o prazo de tempo real está prestes a expirar.

## Observações

- O projeto gera automaticamente `src/parser/scanner.c` e `src/parser/scanner.h` a partir de `src/parser/scanner.l` usando `flex`.
- Se os diretórios `build/` ou `bin/` não existirem, o `make` irá criá-los automaticamente.
- O servidor atual depende de um arquivo de configuração válido passado via `-f`.

## Limpeza

Para remover artefatos de compilação:

```bash
make clean
```

## Licença

Este projeto está disponível apenas para aprendizado e uso educacional.
Uso comercial não é permitido sem autorização prévia.

Se quiser usar todo o código ou parte dele, entre em contato com:

`agsilveira.7@gmail.com`
