# Configuration Reference

This document provides a complete reference for all StreamLog build-time and runtime configuration options. StreamLog favors compile-time configuration over runtime configuration files, which results in smaller binaries and eliminates configuration parsing overhead. Most options are set through Makefile variables or preprocessor macros during the build process. Runtime configuration is limited to the singleton initialization parameters, keeping the library simple and predictable.

## Build Variables

Build variables control compilation behavior and can be set on the command line when invoking `make`. These variables use GNU Make's conditional assignment, meaning command-line values override any defaults set in the Makefile. Variables can also be exported as environment variables before running make, though command-line arguments take precedence. The following subsections organize variables by their functional purpose.

```bash
make VARIABLE=value
```

### Compiler Settings

| Variable | Default | Description |
|----------|---------|-------------|
| `CXX` | `g++` | C++ compiler executable |
| `AR` | `ar` | Archive utility for static libraries |
| `EXTRA_FLAGS` | `--std=c++11` | Additional compiler flags |

**Examples:**

```bash
make CXX=clang++
make EXTRA_FLAGS="-g -O0 --std=c++11"
make CXX=aarch64-linux-gnu-g++  # Cross-compile
```

### Log Level

| Variable | Default | Range | Description |
|----------|---------|-------|-------------|
| `DEBUG_LEVEL` | `1` | 1-6 | Minimum log level to compile |

**Level Mapping:**

| Value | Level | Effect |
|-------|-------|--------|
| 1 | TRACE | All messages compiled |
| 2 | DEBUG | TRACE removed |
| 3 | INFO | TRACE, DEBUG removed |
| 4 | WARN | TRACE, DEBUG, INFO removed |
| 5 | ERROR | Only ERROR, FATAL compiled |
| 6 | FATAL | Only FATAL compiled |

**Examples:**

```bash
make DEBUG_LEVEL=1   # Development (all logs)
make DEBUG_LEVEL=3   # Production (INFO and above)
make DEBUG_LEVEL=6   # Minimal (FATAL only)
```

### Theme

| Variable | Default | Description |
|----------|---------|-------------|
| `THEME` | `default` | Color theme (`default`, `rose_pine_moon`) |

**Examples:**

```bash
make THEME=rose_pine_moon   # Rosé Pine Moon palette
make THEME=default          # Standard ANSI colors (default)
```

### Installation Paths

| Variable | Default | Description |
|----------|---------|-------------|
| `PREFIX` | `/usr/local` | Installation root directory |
| `DESTDIR` | (empty) | Staging directory for packaging |

**Installation Layout:**

```
$(DESTDIR)$(PREFIX)/
├── lib/
│   ├── streamlog.a
│   └── streamlog.so
└── include/
    └── streamlog.hpp
```

**Examples:**

```bash
make install PREFIX=/usr              # System-wide
make install PREFIX=~/.local          # User-local
make install DESTDIR=/tmp/pkg PREFIX=/usr  # Package staging
```

### Platform Switches

| Variable | Default | Description |
|----------|---------|-------------|
| `aarch64` | `0` | Cross-compile for ARM64 (aarch64) targets |
| `mac` | `0` | Build for macOS (adds .dylib target) |

**Effects:**

| Switch | Compiler | Archiver | Additional Targets |
|--------|----------|----------|-------------------|
| `aarch64=1` | `aarch64-linux-gnu-g++-9` | `aarch64-linux-gnu-ar` | — |
| `mac=1` | (unchanged) | (unchanged) | `streamlog.dylib` |

**Examples:**

```bash
make aarch64=1   # Cross-compile for ARM64 from x86_64 host
make mac=1       # Build on macOS with native .dylib format
```

## Preprocessor Macros

Preprocessor macros provide compile-time configuration that affects which code paths are compiled into the final binary. Unlike runtime configuration, these settings cannot be changed without recompiling. Macros can be defined before including `streamlog.hpp` in source code, or more commonly, passed via compiler flags using the `-D` option. The Makefile handles the most common macros automatically based on build variables.

### Log Level Control

| Macro | Type | Default | Description |
|-------|------|---------|-------------|
| `DEBUG_LEVEL` | Integer | `1` | Compile-time log level threshold |

**Via compiler:**

```bash
g++ -DDEBUG_LEVEL=3 myapp.cpp -llog
```

**Via source:**

```cpp
#define DEBUG_LEVEL 3
#include <streamlog.hpp>
```

### Feature Toggles

| Macro | Type | Default | Description |
|-------|------|---------|-------------|
| `ENABLE_VECTOR_LOGGING` | Flag | Defined | Enable `std::vector` logging |
| `ENABLE_MAP_LOGGING` | Flag | Defined | Enable `std::map` logging |

**Disable container logging:**

```bash
# Remove from MACRO_FLAGS in Makefile, or:
g++ -UENABLE_VECTOR_LOGGING myapp.cpp -llog
```

### Internal Macros

| Macro | Type | Default | Description |
|-------|------|---------|-------------|
| `LOG_FILE` | String | `"output.log"` | Default log file path |
| `LOG_LEVEL` | Enum | (derived) | Runtime log threshold |

These are set internally based on `DEBUG_LEVEL`.

## Compiler Flags

When compiling applications that use StreamLog, certain compiler and linker flags are required or recommended. These flags ensure proper language standard compliance, library linking, and include path resolution. The required flags must be present for successful compilation and linking, while recommended flags enable useful warnings and optimizations. Platform-specific flags handle differences between Linux and macOS dynamic library formats.

### Required Flags

| Flag | Purpose |
|------|---------|
| `--std=c++11` | C++11 standard (minimum required) |
| `-I<path>` | Include path for `streamlog.hpp` |
| `-L<path>` | Library path for linking |
| `-llog` | Link against streamlog |

### Recommended Flags

| Flag | Purpose |
|------|---------|
| `-Wall -Wextra` | Enable warnings |
| `-O2` | Optimization (release) |
| `-g` | Debug symbols (development) |

### Library-Specific Flags

| Flag | When Used | Purpose |
|------|-----------|---------|
| `-fPIC` | Shared library build | Position-independent code |
| `-shared` | Linux shared library | Create `.so` file |
| `-dynamiclib` | macOS dynamic library | Create `.dylib` file |

## Runtime Configuration

StreamLog has minimal runtime configuration since most settings are determined at compile time. This design choice eliminates configuration file parsing, reduces potential failure modes, and keeps the library lightweight. The only runtime configuration occurs during singleton initialization, where the log file path and console output setting are specified. These parameters are fixed for the lifetime of the application once the singleton is initialized.

### Singleton Initialization

```cpp
// Configure on first call
StreamLog& logger = StreamLog::instance(
    "path/to/logfile.log",  // Log file path
    true                     // Enable console output
);
```

**Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| File path | `std::string` | Where to write log file |
| Console output | `bool` | Also write to stderr with colors |

**Note:** Parameters only take effect on first call. Subsequent calls return existing instance.

### Log File Behavior

| Behavior | Setting |
|----------|---------|
| File mode | Append |
| Directory creation | Automatic (recursive) |
| Color codes | Written to file (identical to console output) |
| Flush | After each write |

### Console Output Behavior

| Behavior | Setting |
|----------|---------|
| Stream | `stderr` |
| Colors | ANSI escape codes |
| Flush | Immediate |

## Environment Variables

StreamLog does not read environment variables directly, consistent with its philosophy of compile-time configuration. However, standard system environment variables affect how the operating system loads the library and how terminals render colored output. These are operating system concerns rather than StreamLog configuration, but understanding them helps when troubleshooting library loading issues or color display problems. The following table lists the relevant system environment variables.

| Variable | Effect |
|----------|--------|
| `LD_LIBRARY_PATH` | Library search path (Linux) |
| `DYLD_LIBRARY_PATH` | Library search path (macOS) |
| `TERM` | Terminal type (affects color support) |

## Configuration Examples

These examples show common configuration combinations for different use cases. Each example represents a typical deployment scenario with appropriate settings for that context. Development builds prioritize debuggability with all log levels and debug symbols, while production builds optimize for performance with higher log thresholds and compiler optimizations. Specialized builds handle cross-compilation and minimal footprint requirements.

### Development Build

```bash
make DEBUG_LEVEL=1 EXTRA_FLAGS="-g -O0 --std=c++11"
```

All log levels, debug symbols, no optimization.

### Production Build

```bash
make DEBUG_LEVEL=3 EXTRA_FLAGS="-O2 --std=c++11"
```

INFO and above, optimized.

### Minimal Build

```bash
make DEBUG_LEVEL=6 EXTRA_FLAGS="-Os --std=c++11"
```

FATAL only, size-optimized.

### Cross-Compilation

```bash
make CXX=aarch64-linux-gnu-g++ AR=aarch64-linux-gnu-ar
```

### Static-Only Build

Edit Makefile to remove shared library target, or:

```bash
make build/lib/streamlog.a
```
