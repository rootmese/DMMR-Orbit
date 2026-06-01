# DMMR Orbit

DMMR Orbit is a C network server/daemon for session management and traffic routing, featuring a trunk plugin, scheduling, and support for running in the foreground or as a daemon. The project also serves as a real-time emulator, using I/O callbacks to simulate runtime behavior.

## Overview

The project implements a server called `dmmr_orbit` that:
- reads configuration from a file
- starts a custom DMMR network server
- supports running in the foreground
- supports running as a daemon
- uses a Flex parser for configuration
- keeps logs via a ring buffer component
- emulates real-time behavior using I/O callbacks and session scheduling

## Repository Structure

- `src/` - main source code
  - `main.c` - program entry point
  - `daemon/` - control for running as a daemon
  - `server/` - main server logic
  - `parser/` - configuration parsing and reload
  - `socket/` - TCP/UDP socket abstraction
  - `session/` - session connection management
  - `scheduler/` - runtime scheduling logic
  - `plugin/` - trunk plugin and session integration
  - `circle_buffer/` - circular communication buffer
  - `log/` - logging and log handling
  - `utils/` - auxiliary utilities
- `etc/` - configuration files
- `makefile` - build system

## Requirements

- GCC with GNU11 support
- `flex`
- POSIX system (Linux/Unix)
- `librt` library

## Build

From the project root directory:

```bash
make debug
```

For a release build:

```bash
make release
```

The generated binary will be located at `bin/dmmr_orbit`.

## Usage

The binary accepts the following parameters:

- `-f <file.conf>` - path to the configuration file
- `-d 0|1` - 0 = run in foreground, 1 = run as a daemon

### Run in foreground

```bash
./bin/dmmr_orbit -f etc/dmmr_orbit.conf -d 0
```

### Run as daemon

```bash
./bin/dmmr_orbit -f etc/dmmr_orbit.conf -d 1
```

## Configuration

The default example configuration file is `etc/dmmr_orbit.conf`. It includes parameters such as:

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

The configuration file defines the values used by the server, the parser, and the scheduler.

## Plugins and Real-Time Emulation

DMMR Orbit uses a simple plugin interface based on callbacks, designed for real-time emulation behavior.

The main interface is in `src/plugin/include/dmmr_plugin.h` and defines:

- `struct dmmr_plugin` with:
  - `void (*load)(void);`
  - `void (*reload)(void);`
- `struct dmmr_plugin *new_dmmr_plugin(struct dmmr_session_connection_manager*, struct cfg_server_server*);`

In the server, the plugin is instantiated and loaded in `src/server/dmmr_server.c`:

```c
plugin = new_dmmr_plugin(session_manager, &cfg);
plugin->load();
```

The current implementation in `src/plugin/dmmr_trunk_plugin.c` integrates with the session manager and registers network callbacks for accept, connect, dispatch, and close events.

Because this is a real-time emulator, the plugin operations are not called in a traditional synchronous manner: the plugin uses callbacks to react to I/O events and keep session logic inside the runtime loop.

This means:

- the server creates and initializes the plugin;
- the plugin registers network event handlers;
- the plugin manages real-time sessions via `accept`, `connect`, `dispatch`, and `close` callbacks.

This architecture ensures that DMMR Orbit can behave like a real-time emulator, reacting to network events as they occur.

The internal scheduler also supports preemptive deadline scheduling, with a configurable limit (`SCHEDULER_PREEMPTIVE_DEADLINE`). The sending logic is delegated to a scheduler thread that triggers dispatch when a real-time deadline is about to expire.

## Notes

- The project automatically generates `src/parser/scanner.c` and `src/parser/scanner.h` from `src/parser/scanner.l` using `flex`.
- If the `build/` or `bin/` directories do not exist, `make` will create them automatically.
- The current server requires a valid configuration file passed via `-f`.

## Clean

To remove build artifacts:

```bash
make clean
```

## License

This project is provided for learning and educational use only.
Commercial use is not permitted without prior authorization.

If you want to use all or part of the code, contact:

`agsilveira.7@gmail.com`

---

Original README (Portuguese): see `README.md` in this repository.
