# DMMR Orbit

DMMR Orbit é um servidor/daemon em C para gerenciamento de sessões e roteamento de tráfego de rede com plugin de tronco, escalonamento e suporte a operações em foreground ou daemon.

## Visão geral

O projeto implementa um servidor chamado `dmmr_orbit` que:
- lê uma configuração de arquivo
- inicia um servidor DMMR customizado
- suporta execução em foreground
- suporta execução como daemon
- usa um parser Flex para configuração
- mantém logs via componente de ring buffer

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
