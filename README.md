# StreamLog

A lightweight, header-friendly C++11 logging library with stream-based API, colored output, and compile-time log level filtering.

[![CI](https://github.com/Derrekito/StreamLog/actions/workflows/ci.yml/badge.svg)](https://github.com/Derrekito/StreamLog/actions/workflows/ci.yml)
[![Release](https://github.com/Derrekito/StreamLog/actions/workflows/release.yml/badge.svg)](https://github.com/Derrekito/StreamLog/releases)

## Features

- **Stream-based API** - Familiar C++ streaming: `log(INFO) << "value: " << x`
- **Six log levels** - TRACE, DEBUG, INFO, WARN, ERROR, FATAL with colored output
- **Compile-time filtering** - Disabled log levels compiled out entirely
- **Zero dependencies** - Pure C++11 standard library
- **STL container support** - Log vectors and maps directly
- **Thread-safe singleton** - Meyer's singleton pattern (C++11)
- **Extensible** - Inherit and override formatting/timestamps
- **Cross-platform** - Linux, macOS (x86_64, ARM64)

## Quick Start

```cpp
#include <streamlog.hpp>

int main() {
    log(INFO) << "Server started on port " << 8080;
    log(WARN) << "Connection timeout after " << 30 << "s";
    
    std::vector<int> data = {1, 2, 3};
    log(DEBUG) << "Processing: " << data;
    
    return 0;
}
```

**Output** (with colors):
```
1747182400 [INFO] Server started on port 8080
1747182400 [WARN] Connection timeout after 30s
1747182400 [DEBUG] Processing: [1, 2, 3]
```

## Installation

### From Pre-built Release

Download for your platform from [Releases](https://github.com/Derrekito/StreamLog/releases):

```bash
# Linux
curl -LO https://github.com/Derrekito/StreamLog/releases/download/v1.0.0/streamlog-linux-x86_64.tar.gz
tar xzf streamlog-linux-x86_64.tar.gz
sudo cp -r streamlog-linux-x86_64/lib/* /usr/local/lib/
sudo cp -r streamlog-linux-x86_64/include/* /usr/local/include/
sudo ldconfig

# macOS
curl -LO https://github.com/Derrekito/StreamLog/releases/download/v1.0.0/streamlog-macos-x86_64.tar.gz
tar xzf streamlog-macos-x86_64.tar.gz
sudo cp -r streamlog-macos-x86_64/lib/* /usr/local/lib/
sudo cp -r streamlog-macos-x86_64/include/* /usr/local/include/
```

### Build from Source

```bash
git clone https://github.com/Derrekito/StreamLog.git
cd StreamLog
make
sudo make install PREFIX=/usr/local
```

**Platform-specific builds:**
```bash
make mac=1      # macOS: build .dylib
make aarch64=1  # ARM64 cross-compile
```

## Usage

### Basic Logging

```cpp
#include <streamlog.hpp>

// Default: logs to output.log, no console output
log(ERROR) << "Database connection failed";

// Custom configuration (call once before first log)
StreamLog::instance("myapp.log", true);  // custom file, enable console
log(INFO) << "Application started";
```

### Log Levels

| Level | Color | Use Case |
|-------|-------|----------|
| `TRACE` | Gray | Function entry/exit, variable dumps |
| `DEBUG` | Blue | Debug information during development |
| `INFO` | Green | Normal operational messages |
| `WARN` | Yellow | Warning conditions, recoverable errors |
| `ERROR` | Red | Error conditions, failed operations |
| `FATAL` | Red (bold) | Critical failures requiring termination |

### Compile-Time Filtering

Set `DEBUG_LEVEL` to remove lower-priority logs from binary:

```bash
make DEBUG_LEVEL=3  # Only INFO, WARN, ERROR, FATAL (no TRACE/DEBUG)
```

```cpp
// In your code:
log(TRACE) << "This call is compiled out if DEBUG_LEVEL > 1";
log(INFO)  << "This always executes if DEBUG_LEVEL <= 3";
```

**Debug levels:**
- `1` = TRACE and above (default)
- `2` = DEBUG and above
- `3` = INFO and above
- `4` = WARN and above
- `5` = ERROR and above
- `6` = FATAL only

### STL Container Logging

Enable at compile time:

```bash
make                # vector and map support enabled by default
```

```cpp
std::vector<int> nums = {10, 20, 30};
log(INFO) << "Numbers: " << nums;  // [10, 20, 30]

std::map<std::string, int> ages = {{"Alice", 30}, {"Bob", 25}};
log(DEBUG) << "Ages: " << ages;    // {Alice: 30, Bob: 25}
```

### Custom Formatting

Inherit and override:

```cpp
class CustomLogger : public StreamLog {
public:
    CustomLogger() : StreamLog("custom.log", true) {}
    
    std::string getTimestamp() const override {
        return "2026-05-14 10:30:00";  // Your format
    }
    
    std::stringstream buildLog(const std::string& message) const override {
        std::stringstream ss;
        ss << "[" << getTimestamp() << "] " << message << std::endl;
        return ss;
    }
};
```

### Themes

```bash
make THEME=rose_pine_moon   # Rosé Pine Moon palette
make THEME=default          # Standard ANSI colors
```

Add your own: create `include/themes/yourtheme.hpp` with color definitions.

## Building Your Application

```bash
# Dynamic linking (recommended)
g++ -std=c++11 myapp.cpp -lstreamlog -o myapp

# Static linking
g++ -std=c++11 myapp.cpp -L/path/to/lib -lstreamlog -static -o myapp

# With custom flags
g++ -std=c++11 -I/usr/local/include myapp.cpp -L/usr/local/lib -lstreamlog -o myapp
```

## Advanced Configuration

### Build Options

```bash
make CXX=clang++               # Use clang instead of g++
make EXTRA_FLAGS="--std=c++17" # C++17 mode
make PREFIX=/opt/local install # Install to /opt/local
```

### Disable Container Logging

```bash
# Edit Makefile, remove:
# MACRO_FLAGS += -DENABLE_VECTOR_LOGGING
# MACRO_FLAGS += -DENABLE_MAP_LOGGING
make clean && make
```

## Examples

See [`examples/`](examples/) directory:

- [`streamlog_example.cpp`](examples/streamlog_example.cpp) - Basic usage
- [`decorator_example.cpp`](examples/decorator_example.cpp) - Custom formatter inheritance

Build examples:
```bash
cd examples
g++ -std=c++11 -I../include -L../build/lib streamlog_example.cpp -lstreamlog -o streamlog_example
./streamlog_example
```

## CI/CD

- **CI**: Automatically builds on Ubuntu/macOS with g++/clang++ across C++11/14/17
- **CD**: Automated releases with pre-built binaries on version tags

See [`.github/workflows/`](.github/workflows/).

## Requirements

- **Compiler**: g++ or clang++ with C++11 support
- **OS**: Linux, macOS (Windows via WSL or MinGW)
- **Dependencies**: None (standard library only)

## License

MIT License - see [LICENSE](LICENSE) file.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing`)
5. Open a Pull Request

## Support

- **Issues**: [GitHub Issues](https://github.com/Derrekito/StreamLog/issues)
- **Releases**: [GitHub Releases](https://github.com/Derrekito/StreamLog/releases)
- **CI Status**: [![CI](https://github.com/Derrekito/StreamLog/actions/workflows/ci.yml/badge.svg)](https://github.com/Derrekito/StreamLog/actions/workflows/ci.yml)
