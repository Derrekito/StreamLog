# API Reference

This document provides the complete API reference for StreamLog, covering all public functions, classes, enumerations, and preprocessor macros. The API is designed to be minimal yet sufficient for common logging needs, with most users only needing the global `log()` function and the `LogLevel` enumeration. Advanced users who need customization can access the full `StreamLog` class interface and its virtual methods. Each section includes function signatures, parameter descriptions, return values, and usage examples.

## Global Function

The primary interface for logging is a single global function that creates log statements. This function provides the familiar `log(LEVEL) << message` syntax that most users will interact with. Internally, it delegates to the singleton `StreamLog` instance and returns a temporary `LogStatement` object that accumulates the message and writes it when destroyed. For most use cases, this is the only API you need to know.

### log()

```cpp
StreamLog::LogStatement log(LogLevel level);
```

Creates a log statement at the specified severity level.

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `level` | `LogLevel` | Severity level for this message |

**Returns:** `LogStatement` object for stream chaining.

**Example:**

```cpp
log(INFO) << "Server started on port " << port;
```

**Notes:**

- The returned `LogStatement` writes to the log when destroyed (end of statement)
- Messages below `DEBUG_LEVEL` threshold are compiled out

---

## Enumerations

StreamLog defines one enumeration for specifying log severity levels. The `LogLevel` enum provides six ordered values from most verbose (TRACE) to least verbose (FATAL). These values are used both at runtime to tag log messages and at compile time via the `DEBUG_LEVEL` macro to filter out unwanted log levels. The numeric values of the enum are significant, as they determine the filtering threshold.

### LogLevel

```cpp
enum LogLevel {
    TRACE,  // 0 - Most verbose
    DEBUG,  // 1
    INFO,   // 2
    WARN,   // 3
    ERROR,  // 4
    FATAL   // 5 - Least verbose
};
```

Severity levels for log messages, ordered from most to least verbose.

---

## Classes

The library provides three types: the main `StreamLog` class, its nested `LogStatement` class for stream operations, and the `LibColor` struct for color constants. The `StreamLog` class implements the singleton pattern and manages configuration, file output, and console output. The `LogStatement` inner class provides the stream interface via `operator<<` and uses RAII to commit log entries when the statement ends. The `LibColor` struct is a simple collection of ANSI escape code constants used for terminal coloring.

### StreamLog

Main logging class. Singleton pattern.

#### Public Methods

##### instance()

```cpp
static StreamLog& instance(const std::string& fileName, bool consoleOutput = false);
```

Returns the singleton logger instance.

**Parameters:**

| Name | Type | Default | Description |
|------|------|---------|-------------|
| `fileName` | `const std::string&` | (required) | Path to log file |
| `consoleOutput` | `bool` | `false` | Enable stderr output with colors |

**Returns:** Reference to the singleton `StreamLog` instance.

**Notes:**

- First call initializes the singleton with provided parameters
- Subsequent calls return existing instance (parameters ignored)
- Initialization is not thread-safe. Concurrent first calls may produce multiple instances.

---

##### getLogStatement()

```cpp
LogStatement& getLogStatement(LogLevel level);
```

Creates a `LogStatement` for the specified level.

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `level` | `LogLevel` | Severity level |

**Returns:** Reference to a `LogStatement` object.

**Notes:**

- Prefer using global `log()` function instead
- Used internally and for custom logger implementations

---

#### Virtual Methods

Override these in derived classes for customization.

##### getTimestamp()

```cpp
virtual std::string getTimestamp();
```

Generates the timestamp string for log entries.

**Returns:** Timestamp as string.

**Default:** UNIX epoch seconds (e.g., `"1732835400"`).

**Example Override:**

```cpp
std::string getTimestamp() override {
    return "2024-01-15T10:30:00";
}
```

---

##### buildLog()

```cpp
virtual std::stringstream buildLog(const std::string& message);
```

Constructs the formatted log entry.

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `message` | `const std::string&` | The log message content |

**Returns:** `std::stringstream` containing the formatted entry.

**Default Format:** `<timestamp> [LEVEL] <message>\n`

**Example Override:**

```cpp
std::stringstream buildLog(const std::string& message) override {
    std::stringstream ss;
    ss << "{\"msg\":\"" << message << "\"}\n";
    return ss;
}
```

---

#### Private Members

##### m_level

```cpp
LogLevel m_level;
```

Current log level being processed. This member is private and is not accessible from derived class overrides.

---

### StreamLog::LogStatement

Inner class implementing the stream interface. Created per log statement, writes on destruction.

#### Operators

##### operator<< (template)

```cpp
template<typename T>
LogStatement& operator<<(const T& value);
```

Appends a value to the log message.

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `value` | `const T&` | Value to append (any type with `operator<<`) |

**Returns:** Reference to `this` for chaining.

**Example:**

```cpp
log(INFO) << "Count: " << 42 << ", Name: " << name;
```

---

##### operator<< (C-string)

```cpp
LogStatement& operator<<(const char* value);
```

Appends a C-string to the log message.

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `value` | `const char*` | Null-terminated string |

**Returns:** Reference to `this` for chaining.

---

##### operator<< (manipulator)

```cpp
LogStatement& operator<<(std::ostream& (*manipulator)(std::ostream&));
```

Supports stream manipulators.

**Parameters:**

| Name | Type | Description |
|------|------|-------------|
| `manipulator` | function pointer | Stream manipulator (e.g., `std::endl`) |

**Returns:** Reference to `this` for chaining.

**Example:**

```cpp
log(INFO) << "Line 1" << std::endl << "Line 2";
```

---

### LibColor

Struct containing ANSI color code constants.

#### Constants

| Name | Type | Value | Description |
|------|------|-------|-------------|
| `TraceColor` | `static const std::string` | `"\033[1;30m"` | Bright black (gray) |
| `DebugColor` | `static const std::string` | `"\033[1;34m"` | Bright blue |
| `InfoColor` | `static const std::string` | `"\033[1;32m"` | Bright green |
| `WarnColor` | `static const std::string` | `"\033[1;93m"` | Bright yellow |
| `ErrorColor` | `static const std::string` | `"\033[1;31m"` | Bright red |
| `FatalColor` | `static const std::string` | `"\033[31;1m"` | Bright red |
| `reset` | `static const std::string` | `"\033[0m"` | Reset to default |

**Usage:**

```cpp
std::cerr << LibColor::ErrorColor << "Error!" << LibColor::reset;
```

---

## Container Operators

StreamLog provides optional stream operators for STL containers, enabled via preprocessor macros at compile time. These operators allow vectors and maps to be logged directly without manual formatting, which is particularly useful during debugging. The operators are enabled by default in the Makefile build, but can be disabled by undefining the corresponding macros. Container contents are formatted in a human-readable format with brackets for vectors and braces for maps.

When `ENABLE_VECTOR_LOGGING` is defined:

```cpp
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec);
```

Outputs vector as `[item1, item2, item3]`.

When `ENABLE_MAP_LOGGING` is defined:

```cpp
template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& map);
```

Outputs map as `{key1: value1, key2: value2}`.

---

## Preprocessor Macros

These macros control compile-time behavior and feature toggles. The most important is `DEBUG_LEVEL`, which determines the minimum log level that gets compiled into the binary. Macros prefixed with `ENABLE_` toggle optional features like container logging support. These macros are typically set via compiler flags in the Makefile, but can also be defined in source code before including the header.

### DEBUG_LEVEL

Controls compile-time log level filtering.

| Value | Minimum Level |
|-------|---------------|
| 1 | TRACE |
| 2 | DEBUG |
| 3 | INFO |
| 4 | WARN |
| 5 | ERROR |
| 6 | FATAL |

**Default:** `1` (all levels enabled)

### ENABLE_VECTOR_LOGGING

Enables `operator<<` for `std::vector<T>`.

**Default:** Defined in Makefile

### ENABLE_MAP_LOGGING

Enables `operator<<` for `std::map<K,V>`.

**Default:** Defined in Makefile

### LOG_FILE

Default log file path used internally.

**Default:** `"output.log"`
