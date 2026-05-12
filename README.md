# StreamLog

StreamLog is a lightweight C++ logging library designed for simplicity and performance. It provides a stream-based API familiar to C++ developers, colored console output for quick visual scanning, and compile-time log level filtering that eliminates disabled log statements entirely from the compiled binary. The library requires only C++11 and has no external dependencies, making it easy to integrate into any project.

## Features

- Six log levels: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- Stream-based fluent API: `log(LEVEL) << "message" << value`
- Colored console output (ANSI)
- File output with automatic directory creation
- Compile-time filtering via `DEBUG_LEVEL`
- STL container logging (vectors, maps)
- Extensible via inheritance

## Quick Start

```cpp
#include <streamlog.hpp>

int main() {
    log(INFO)  << "Server started on port " << 8080;
    log(WARN)  << "Connection timeout";
    log(ERROR) << "Failed to write to database";
    return 0;
}
```

By default, `log()` writes to `output.log` in the current directory. To use a custom path or change console output, call `StreamLog::instance()` before the first `log()` call:

```cpp
StreamLog::instance("myapp.log", true);  // custom path, console output enabled
```

## Build & Install

```bash
make                      # Build
sudo make install         # Install to /usr/local
```

Compile your application:

```bash
g++ -std=c++11 myapp.cpp -llog -o myapp
```

### Platform Options

```bash
make mac=1      # macOS: adds .dylib alongside .a and .so
make aarch64=1  # ARM64: cross-compile with aarch64 toolchain
```

### Color Theme

```bash
make THEME=rose_pine_moon   # Rosé Pine Moon color theme
make THEME=default          # Standard ANSI colors (default)
```

See [Building](docs/development/building.md#platform-specific-builds) for detailed platform configuration.

## Log Levels

| Level | Color | Use |
|-------|-------|-----|
| TRACE | Gray | Detailed debugging |
| DEBUG | Blue | Debug information |
| INFO | Green | Operational messages |
| WARN | Yellow | Warning conditions |
| ERROR | Red | Error conditions |
| FATAL | Red | Critical failures |

Control compile-time filtering:

```bash
make DEBUG_LEVEL=3  # INFO and above only
```

## Documentation

- **[Architecture](docs/architecture.md)** - Design and internals
- **Usage**
  - [Log Levels](docs/usage/log-levels.md) - Levels, filtering, colors
  - [Customization](docs/usage/customization.md) - Custom formatters
- **Development**
  - [Building](docs/development/building.md) - Build system details
- **Reference**
  - [API](docs/reference/api.md) - Complete API reference
  - [Configuration](docs/reference/configuration.md) - Build variables
  - [Troubleshooting](docs/reference/troubleshooting.md) - Common issues

Full manual: generate with `cd manual && make pdf` (requires pandoc and LaTeX).

## License

See LICENSE file for details.
