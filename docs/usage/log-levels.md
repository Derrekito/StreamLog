# Log Levels

StreamLog provides six severity levels for categorizing log messages, enabling developers to control the verbosity and importance of logged information. Each level represents a different category of information, from fine-grained debugging traces to critical failure notifications. Understanding when to use each level and how compile-time filtering works is essential for effective logging. This document covers level semantics, usage guidelines, compile-time filtering via `DEBUG_LEVEL`, and the ANSI color scheme used for console output.

## Level Hierarchy

Choosing the appropriate log level is important for effective debugging and monitoring. The level you assign to a message determines whether it appears in the output based on the compile-time `DEBUG_LEVEL` threshold. Higher numeric values represent more severe (less verbose) levels, with TRACE being the most verbose and FATAL being reserved for unrecoverable errors. Log levels are ordered from most verbose (TRACE) to least verbose (FATAL):

| Level | Value | Purpose |
|-------|-------|---------|
| TRACE | 0 | Fine-grained debugging: function entry/exit, variable dumps |
| DEBUG | 1 | General debugging information |
| INFO  | 2 | Normal operational messages |
| WARN  | 3 | Potentially harmful situations |
| ERROR | 4 | Error events that might allow continued operation |
| FATAL | 5 | Severe errors causing premature termination |

## Choosing the Right Level

Selecting the appropriate level for each message helps maintain useful logs without excessive noise. Overly verbose logging at high levels (WARN, ERROR) clutters logs and makes it harder to identify genuine problems. Conversely, using low levels (TRACE, DEBUG) for important information means those messages may be filtered out in production builds. Use the following guidelines for each level.

### TRACE
Use for detailed diagnostic information during development:

```cpp
log(TRACE) << "Entering processData(), size=" << data.size();
log(TRACE) << "Loop iteration " << i << ", value=" << values[i];
log(TRACE) << "Exiting processData()";
```

### DEBUG
Use for information useful during debugging:

```cpp
log(DEBUG) << "Configuration loaded: " << configPath;
log(DEBUG) << "Cache hit ratio: " << hits << "/" << total;
```

### INFO
Use for general operational events:

```cpp
log(INFO) << "Server started on port " << port;
log(INFO) << "Processing batch " << batchId << " complete";
log(INFO) << "User " << userId << " logged in";
```

### WARN
Use for potentially problematic situations:

```cpp
log(WARN) << "Configuration file not found, using defaults";
log(WARN) << "Memory usage at " << percent << "%, consider cleanup";
log(WARN) << "Retry attempt " << attempt << " of " << maxRetries;
```

### ERROR
Use for error conditions that don't require termination:

```cpp
log(ERROR) << "Failed to connect to database: " << error;
log(ERROR) << "Invalid input: " << input;
log(ERROR) << "File not found: " << path;
```

### FATAL
Use for unrecoverable errors:

```cpp
log(FATAL) << "Out of memory, cannot continue";
log(FATAL) << "License expired, terminating";
log(FATAL) << "Critical system failure: " << reason;
```

## Compile-Time Filtering

One of StreamLog's key features is compile-time log level filtering through the `DEBUG_LEVEL` macro. Unlike runtime filtering where all logging code exists in the binary but is conditionally skipped, compile-time filtering completely removes log statements below the threshold from the compiled code. This results in zero runtime overhead for disabled levels since the code simply doesn't exist in the binary. Messages below the threshold are eliminated at compile time, and the compiler can optimize away the entire log statement including any string formatting or function calls in the message.

### Setting DEBUG_LEVEL

Set during compilation:

```bash
make DEBUG_LEVEL=3  # INFO and above
```

Or define before including the header:

```cpp
#define DEBUG_LEVEL 4  // WARN and above
#include <streamlog.hpp>
```

### Threshold Mapping

| DEBUG_LEVEL | Minimum Level | Levels Shown |
|-------------|---------------|--------------|
| 1 | TRACE | TRACE, DEBUG, INFO, WARN, ERROR, FATAL |
| 2 | DEBUG | DEBUG, INFO, WARN, ERROR, FATAL |
| 3 | INFO | INFO, WARN, ERROR, FATAL |
| 4 | WARN | WARN, ERROR, FATAL |
| 5 | ERROR | ERROR, FATAL |
| 6 | FATAL | FATAL only |

### Production vs Development

Typical settings:

- **Development**: `DEBUG_LEVEL=1` (show everything)
- **Testing**: `DEBUG_LEVEL=2` (hide TRACE)
- **Production**: `DEBUG_LEVEL=3` or `DEBUG_LEVEL=4` (INFO or WARN and above)

## Console Colors

Visual differentiation of log levels speeds up log analysis, especially when scrolling through large volumes of output. When console output is enabled, each level displays in a distinct color using ANSI escape codes that are widely supported by modern terminals. The color scheme uses intuitive associations: green for informational messages, yellow for warnings, and red for errors.

| Level | Color | ANSI Code |
|-------|-------|-----------|
| TRACE | Gray | `\033[1;30m` |
| DEBUG | Blue | `\033[1;34m` |
| INFO | Green | `\033[1;32m` |
| WARN | Yellow | `\033[1;93m` |
| ERROR | Red | `\033[1;31m` |
| FATAL | Red | `\033[31;1m` |

Note: ANSI escape codes are written to both the console and the log file. To get plain text in the file, post-process with: `sed 's/\x1b\[[0-9;]*m//g' output.log`

### Terminal Compatibility

ANSI colors work in:

- Linux terminals (GNOME Terminal, Konsole, xterm)
- macOS Terminal and iTerm2
- Windows Terminal, PowerShell (Windows 10+)
- VS Code integrated terminal

Colors will not render in:

- Windows Command Prompt (legacy)
- Log file viewers that do not interpret ANSI codes (raw escape sequences will appear as literal text)
- Some CI/CD environments

## Output Format

Understanding the output format helps when parsing logs or integrating with log aggregation systems. The default format is deliberately simple and consistent, making it easy to parse with standard Unix tools like `grep`, `awk`, and `cut`. Each log entry occupies a single line, with fields separated by spaces and the level enclosed in brackets. Log messages are formatted as:

```
<timestamp> [LEVEL] <message>
```

Example output:

```
1732835400 [TRACE] Entering main()
1732835400 [DEBUG] Config path: /etc/app.conf
1732835401 [INFO] Server started
1732835402 [WARN] High memory usage detected
1732835403 [ERROR] Connection refused
1732835404 [FATAL] Cannot recover, exiting
```

The timestamp is UNIX epoch seconds. See [Customization](customization.md) for alternative timestamp formats.

## Container Logging

For convenience during debugging, StreamLog can log STL containers directly without requiring manual iteration or string conversion. This feature is enabled by default in the Makefile through preprocessor macros that add `operator<<` overloads for `std::vector` and `std::map`. The output uses readable formats with brackets for vectors and braces for maps. This feature is particularly useful for quickly inspecting container contents during development.

### Vectors

```cpp
std::vector<int> nums = {1, 2, 3, 4, 5};
log(DEBUG) << "Numbers: " << nums;
// Output: Numbers: [1, 2, 3, 4, 5]
```

### Maps

```cpp
std::map<std::string, int> scores = {{"alice", 95}, {"bob", 87}};
log(DEBUG) << "Scores: " << scores;
// Output: Scores: {alice: 95, bob: 87}
```

### Controlling Container Support

Defined in the Makefile by default:

```makefile
MACRO_FLAGS += -DENABLE_VECTOR_LOGGING
MACRO_FLAGS += -DENABLE_MAP_LOGGING
```

To disable, remove these flags or undefine before including:

```cpp
#undef ENABLE_VECTOR_LOGGING
#include <streamlog.hpp>
```
