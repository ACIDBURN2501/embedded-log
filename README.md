
# Log Module for Embedded Systems

This module provides lightweight, robust logging suitable for embedded systems
without traditional stdout, UART, or file-based logging interfaces.
Log entries are stored in a static RAM-based circular buffer and can be inspected
live via JTAG or during post-mortem debugging.

The log API is minimal and suitable for safety-critical or resource-constrained
microcontrollers, with no dynamic memory or OS dependencies.

---

## Features

- RAM-based ring buffer for logging
- Supports log levels: `INFO`, `WARN`, `FAULT`
- Simple, `printf`-style log API: `log_event(level, fmt, ...)`
- `LOG_ONCE` macro to prevent log spam in state machines or tight loops
- Fully defensive: safe against NULL pointers and misuse
- Doxygen-annotated and MISRA C / Linux Kernel style compliant
- Easily integrated as a Meson subproject
- Unit-tested via Unity
- All log messages accessible for inspection with a debugger

---

## Integration
```c
#include "log.h"
```

# Example Usage
```c
// During early init:
log_init(my_timestamp_function);
```

You must provide a monotonic timestamp function; the unit (e.g., milliseconds,
ticks) is user-defined.

```c
void foo(void) {
    log_event(INFO, "Entering foo()");
    if (error) {
        log_event(FAULT, "Error: %d", error);
    }
    LOG_ONCE(WARN, "This warning will only appear once per boot.");
}
```

# Building the Project with Meson

This project uses Meson for building and dependency management.

```c
meson setup builddir
meson compile -C builddir
```


# Running Unit Tests with Meson
Unit tests use Unity (included as a subproject):

```c
meson test -C builddir --print-errorlogs
```

