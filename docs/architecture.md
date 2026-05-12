# Architecture

StreamLog's architecture prioritizes simplicity and performance while providing sufficient flexibility for customization. The library follows established C++ design patterns to deliver a familiar stream-based API that integrates naturally with existing codebases. Understanding the internal design helps when debugging issues, extending functionality through inheritance, or integrating StreamLog into specialized environments. This document covers the core design patterns, class hierarchy, data flow, and extension points that define how StreamLog operates.

## Design Goals

StreamLog was designed with the following goals:

1. **Simplicity**: Minimal API surface, easy to integrate
2. **Performance**: Zero overhead for disabled log levels via compile-time filtering
3. **Familiarity**: Stream-based syntax similar to `std::cout`
4. **Extensibility**: Virtual methods allow customization without modifying core code

## Core Design Patterns

StreamLog employs several well-established design patterns to achieve its goals of simplicity, performance, and extensibility. These patterns work together to provide a clean API while maintaining flexibility for advanced use cases.

### Singleton Pattern

StreamLog uses a singleton pattern for the main logger instance. This provides:

- Global access without passing logger objects
- Single point of configuration (file path, console output)
- Consistent state across the application

```cpp
// First call initializes, subsequent calls return same instance
StreamLog& logger = StreamLog::instance("output.log", true);
```

The singleton is implemented with a static raw pointer initialized to NULL. Initialization is not thread-safe; concurrent first calls can produce multiple instances.

### Fluent Interface (Method Chaining)

The stream insertion operators return references to enable chaining:

```cpp
log(INFO) << "Value: " << x << ", Count: " << n;
```

This is implemented through the `LogStatement` inner class, which accumulates the message in a `std::ostringstream` buffer.

### RAII for Log Commitment

The `LogStatement` object uses RAII (Resource Acquisition Is Initialization) to trigger log writing:

1. `log(LEVEL)` creates a temporary `LogStatement` object
2. `operator<<` calls append to the internal buffer
3. When the statement ends, `LogStatement` destructor fires
4. Destructor calls `commitLog()` which writes to file/console

This ensures logs are written even if an exception occurs during message construction.

## Class Hierarchy

The library consists of three main components: the `StreamLog` class (with its nested `LogStatement` class), the `LibColor` struct for ANSI codes, and the `LogLevel` enumeration. These components work together through well-defined interfaces to provide the logging functionality. The `StreamLog` class manages configuration and output, `LogStatement` handles stream accumulation and RAII-based log commitment, and `LibColor` provides the ANSI escape sequences for terminal coloring. The following diagram shows their relationships and key members.

```{.text .nobreak}
┌─────────────────────────────────────────────────────┐
│                      StreamLog                         │
│  ┌───────────────────────────────────────────────┐  │
│  │              LogStatement                     │  │
│  │  - m_ostringstream : std::ostringstream       │  │
│  │  - m_level : LogLevel                         │  │
│  │  + operator<<(T) : LogStatement&              │  │
│  │  + ~LogStatement() → commitLog()              │  │
│  └───────────────────────────────────────────────┘  │
│                                                     │
│  - m_fileName : std::string                         │
│  - m_consoleOutput : bool                           │
│  - m_level : LogLevel                               │
│                                                     │
│  + instance(fileName, consoleOutput) : StreamLog&      │
│  + getLogStatement(level) : LogStatement            │
│  + virtual getTimestamp() : std::string             │
│  + virtual buildLog(message) : std::stringstream    │
│  - commitLog(message) : void                        │
│  - writeLog() : void                                │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│                    LibColor                         │
│  + TraceColor : static const std::string = "\033[1;30m" │
│  + DebugColor : static const std::string = "\033[1;34m" │
│  + InfoColor  : static const std::string = "\033[1;32m" │
│  + WarnColor  : static const std::string = "\033[1;93m" │
│  + ErrorColor : static const std::string = "\033[1;31m" │
│  + FatalColor : static const std::string = "\033[31;1m" │
│  + reset      : static const std::string = "\033[0m"    │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│               LogLevel (enum)                       │
│  TRACE=0, DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5 │
└─────────────────────────────────────────────────────┘
```

## Data Flow

Understanding how log messages flow through the system helps when debugging issues or extending the library. The flow begins when user code invokes the `log()` function and ends when the message appears in the log file and/or console. Several intermediate steps handle level filtering, message formatting, color application, and output routing. This section traces a log statement from user code through to file and console output.

```{.text .nobreak}
User Code                    StreamLog                         Output
─────────────────────────────────────────────────────────────────────
log(INFO) << "msg"
        │
        ▼
   ┌─────────────┐
   │ log(level)  │ ──► Creates LogStatement with level
   └─────────────┘
        │
        ▼
   ┌─────────────┐
   │ operator<<  │ ──► Appends to internal ostringstream
   └─────────────┘
        │
        ▼
   ┌─────────────┐
   │ ~LogStatement│ ──► Destructor called at statement end
   └─────────────┘
        │
        ▼
   ┌─────────────┐
   │ commitLog() │ ──► Check level vs DEBUG_LEVEL threshold
   └─────────────┘
        │
        ▼ (if level >= threshold)
   ┌─────────────┐
   │ buildLog()  │ ──► Format: timestamp + [LEVEL] + message
   └─────────────┘
        │
        ▼
   ┌─────────────┐      ┌──────────────────────────────┐
   │ writeLog()  │ ───► │ File output (with ANSI codes) │
   └─────────────┘      └──────────────────────────────┘
        │               ┌──────────────────────────────┐
        └─────────────► │ Console/stderr (ANSI codes)  │
                        └──────────────────────────────┘
```

### Compile-Time Filtering

The `DEBUG_LEVEL` macro controls which log levels are compiled:

```cpp
// In streamlog.cpp
#if DEBUG_LEVEL <= 1
    #define LOG_LEVEL TRACE
#elif DEBUG_LEVEL == 2
    #define LOG_LEVEL DEBUG
// ... etc
#endif
```

When a log level is below the threshold, the compiler can optimize away the entire log statement, resulting in zero runtime overhead for disabled levels.

## File Structure

The library source code is organized into two files: a header for the public API and an implementation file. This separation follows the standard C++ header/source convention, allowing users to include only the public interface while keeping implementation details hidden. The header contains all declarations needed by client code, while the source file contains the actual function implementations. This structure also enables independent compilation and helps maintain a clean public API boundary.

```
src/
├── streamlog.hpp    # Public API: LogLevel enum, StreamLog class, LibColor struct
└── streamlog.cpp    # Implementation: singleton, file I/O, formatting
```

### streamlog.hpp Responsibilities

- Define `LogLevel` enumeration
- Declare `StreamLog` class with public interface
- Define `LibColor` struct with ANSI codes
- Declare global `log()` function
- Conditional STL container operator overloads (`ENABLE_VECTOR_LOGGING`, `ENABLE_MAP_LOGGING`)

### streamlog.cpp Responsibilities

- Implement singleton pattern
- Map `DEBUG_LEVEL` macro to runtime threshold
- Implement file output with directory creation
- Implement console output with color codes
- Write identical ANSI-decorated output to both file and console
- Implement timestamp generation

## Extension Points

StreamLog is designed to be extended through inheritance rather than configuration files or runtime settings. This approach leverages C++'s virtual method dispatch to provide customization points that are both type-safe and performant. By subclassing `StreamLog` and overriding the designated virtual methods, developers can customize timestamps, log formats, and output behavior without modifying the library source code. The two primary extension points are `getTimestamp()` for controlling the time format and `buildLog()` for restructuring the entire log entry format.

### getTimestamp()

Override to change timestamp format:

```cpp
class ISOLogger : public StreamLog {
    std::string getTimestamp() override {
        // Return ISO 8601 format instead of UNIX epoch
        return "2024-01-15T10:30:00Z";
    }
};
```

### buildLog()

Override to change entire log format:

```cpp
class JSONLogger : public StreamLog {
    std::stringstream buildLog(const std::string& msg) override {
        std::stringstream ss;
        ss << "{\"ts\":" << getTimestamp()
           << ",\"msg\":\"" << msg << "\"}\n";
        return ss;
    }
};
```

## Thread Safety

StreamLog does **not** provide built-in thread synchronization, a deliberate design choice that keeps the library lightweight and avoids imposing synchronization overhead on single-threaded applications. The singleton is initialized via a raw pointer NULL check and is not thread-safe; concurrent first calls may construct multiple instances. Once initialized, concurrent writes to the log file from multiple threads are also not synchronized, which can result in interleaved or corrupted output. For multi-threaded applications, developers should ensure the singleton is initialized before spawning threads and wrap subsequent log calls with external synchronization as shown below.

```cpp
std::mutex log_mutex;

template<typename... Args>
void safe_log(LogLevel level, Args&&... args) {
    std::lock_guard<std::mutex> lock(log_mutex);
    (log(level) << ... << args);
}
```

## Memory Model

Understanding StreamLog's memory management helps when integrating with memory-constrained systems or debugging memory-related issues. The library favors simplicity over aggressive optimization, using straightforward allocation patterns that are predictable and easy to reason about. Memory usage scales linearly with log message size, and no persistent buffers are maintained beyond the singleton instance itself. The following breakdown covers each component's memory characteristics.

- **Singleton**: Single `StreamLog` instance persists for program lifetime
- **LogStatement**: Temporary object, created and destroyed per log statement
- **File handle**: Opened on each write, closed after (append mode)
- **String buffers**: Stack-allocated `std::ostringstream` in `LogStatement`

The file-per-write approach trades some performance for simplicity and ensures logs are flushed even on abnormal termination.
