# DMMR Orbit

DMMR Orbit is an experimental real-time network emulation framework written in C.

The project was designed to study and validate session-oriented traffic orchestration, QoS strategies, real-time scheduling, and event-driven network processing in heterogeneous environments.

Unlike traditional network daemons that simply react to incoming packets, DMMR Orbit combines session management, callback-based I/O processing, circular buffers, and preemptive deadline scheduling to emulate real-time behavior under controlled conditions.

## Main Features

- Event-driven TCP/UDP network processing
- Session-oriented architecture
- Real-time emulation based on I/O callbacks
- Preemptive deadline scheduler
- Circular buffer communication layer
- Plugin-based runtime integration
- Configuration reload support
- Foreground or daemon execution
- Low-overhead memory management
- Research platform for QoS and traffic orchestration experiments

## Architecture

DMMR Orbit is organized around several core components:

### Session Manager

Responsible for allocating, tracking, and managing active communication sessions.

### Scheduler

Controls runtime execution and dispatch operations using configurable preemptive deadlines.

### Plugin Layer

Provides integration points for custom runtime behavior through callback registration.

### Network Layer

Handles TCP and UDP communication using an event-driven model.

### Circular Buffer

Provides efficient communication between internal processing components with minimal overhead.

### Configuration Parser

Loads and validates runtime configuration files using a Flex-based parser.

## Repository Structure

```text
src/
├── main.c
├── daemon/
├── server/
├── parser/
├── socket/
├── session/
├── scheduler/
├── plugin/
├── circle_buffer/
├── log/
└── utils/

etc/
└── dmmr_orbit.conf

makefile
```

## Requirements

- GCC with GNU11 support
- Flex
- POSIX-compatible operating system (Linux/Unix)

## Build

Debug build:

```bash
make debug
```

Release build:

```bash
make release
```

Generated binary:

```text
bin/dmmr_orbit
```

## Running

Foreground mode:

```bash
./bin/dmmr_orbit -f etc/dmmr_orbit.conf -d 0
```

Daemon mode:

```bash
./bin/dmmr_orbit -f etc/dmmr_orbit.conf -d 1
```

## Command Line Parameters

| Parameter | Description |
|------------|------------|
| -f | Configuration file |
| -d 0 | Run in foreground |
| -d 1 | Run as daemon |

Example:

```bash
./bin/dmmr_orbit -f etc/dmmr_orbit.conf -d 0
```

## Configuration

The configuration file controls scheduler behavior, session limits, buffer sizes, network endpoints, and runtime parameters.

Examples of configurable values:

- SCHEDULER_PREEMPTIVE_DEADLINE
- SLEEP_TIME
- SESSION_SIZE
- CIRCLE_BUFFER_SIZE
- MAX_PORTS
- REAL_TIME_DEAD_LINE
- REAL_TIME_USER_DEFINED
- TRUNK_ACCEPT_URI
- TRUNK_DISPATCH_URI
- RB_LOG_LEVEL

Default configuration:

```text
etc/dmmr_orbit.conf
```

## Real-Time Emulation Model

DMMR Orbit was created as a real-time emulation platform.

Instead of processing traffic through synchronous request/response flows, runtime behavior is driven by network events and callback execution.

The system operates through:

1. Network event detection
2. Session association
3. Callback execution
4. Scheduler evaluation
5. Dispatch processing
6. Deadline enforcement

This architecture allows experiments involving:

- QoS strategies
- Session scheduling
- Traffic shaping
- Runtime orchestration
- Event-driven communication systems

## Plugin System

Plugins are loaded by the server and register callbacks responsible for handling runtime events.

The default trunk plugin integrates with:

- Accept events
- Connect events
- Dispatch events
- Close events

This enables custom traffic processing without modifying the core runtime.

## Research Goals

DMMR Orbit serves as a research and experimentation platform for:

- Real-time emulation
- Session-oriented networking
- Scheduling algorithms
- QoS mechanisms
- Event-driven architectures
- Runtime traffic orchestration

## Clean Build Artifacts

```bash
make clean
```

## License

## License

This project is licensed under the terms described in the `LICENSE` file.

Copyright (c) 2026 by Alessadro Silveira

The source code is available for learning, study, and educational purposes.

For licensing inquiries:

agsilveira.7@gmail.com