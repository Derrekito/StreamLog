# Customization

StreamLog provides extension points through inheritance rather than configuration files, allowing compile-time customization that incurs no runtime configuration parsing overhead. The library exposes virtual methods that subclasses can override to modify timestamp formats, log entry structure, and output behavior. This approach leverages C++'s type system to ensure customizations are type-safe and integrates naturally with the existing API. This document covers how to customize timestamps, log formats, and create specialized loggers by subclassing the `StreamLog` class.

## Extension Points

StreamLog exposes two virtual methods that can be overridden in subclasses to customize behavior. The `getTimestamp()` method controls the format of the timestamp that appears at the beginning of each log entry. The `buildLog()` method provides complete control over the entire log entry format, including the ability to output structured formats like JSON. Override only what you need; most customizations only require one of these methods.

| Method | Purpose | Default Behavior |
|--------|---------|------------------|
| `getTimestamp()` | Generate timestamp string | Returns UNIX epoch seconds |
| `buildLog(message)` | Format the complete log entry | `timestamp [LEVEL] message\n` |

**Note:** The current log level member `m_level` and the helper `levelToString()` are both private in the base class and are not accessible from derived class overrides. Override methods that need the level string must either maintain their own level tracking or require `levelToString()` to be made protected in the base class.

## Custom Timestamp Formats

The default timestamp is UNIX epoch seconds (e.g., `1732835400`), which is compact and ideal for programmatic parsing but not human-readable. Many applications prefer ISO 8601 format, local time with milliseconds, or other domain-specific formats. Overriding `getTimestamp()` is straightforward and only requires returning a string in your desired format. The following examples demonstrate common timestamp customizations.

### ISO 8601 Timestamp

```cpp
#include <streamlog.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

class ISOLogger : public StreamLog {
public:
    ISOLogger(const std::string& file, bool console = true)
        : StreamLog(file, console) {}

    std::string getTimestamp() override {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S");
        return ss.str();
    }
};
```

Output:
```
2024-01-15T10:30:45 [INFO] Server started
```

### Timestamp with Milliseconds

```cpp
class PreciseLogger : public StreamLog {
public:
    PreciseLogger(const std::string& file, bool console = true)
        : StreamLog(file, console) {}

    std::string getTimestamp() override {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
};
```

Output:
```
10:30:45.123 [DEBUG] Processing request
```

## Custom Log Formats

For more extensive customization, override `buildLog()` to change the entire log entry structure. This method receives the user's message and controls the output format. Note that `m_level` and `levelToString()` are private in the base class and are not accessible from `buildLog()` overrides without modifying the base class. Common use cases include JSON output for log aggregation systems, syslog-compatible format for integration with system logging, and structured formats that include contextual metadata. The following examples demonstrate several format customizations; each notes where private member access would be required.

### JSON Format

```cpp
class JSONLogger : public StreamLog {
public:
    JSONLogger(const std::string& file, bool console = true)
        : StreamLog(file, console) {}

    std::stringstream buildLog(const std::string& message) override {
        std::stringstream ss;
        ss << "{"
           << "\"timestamp\":" << getTimestamp() << ","
           // "level" field omitted: levelToString() and m_level are private
           // and not accessible here without modifying the base class
           << "\"message\":\"" << escapeJson(message) << "\""
           << "}" << std::endl;
        return ss;
    }

private:
    std::string escapeJson(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"':  result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n";  break;
                default:   result += c;
            }
        }
        return result;
    }
};
```

Output:
```json
{"timestamp":1732835400,"message":"Server started"}
```

### Syslog-Style Format

```cpp
class SyslogLogger : public StreamLog {
public:
    SyslogLogger(const std::string& file, bool console = true)
        : StreamLog(file, console), m_hostname(getHostname()) {}

    std::stringstream buildLog(const std::string& message) override {
        std::stringstream ss;
        ss << "<" << getSyslogPriority() << ">"
           << getTimestamp() << " "
           << m_hostname << " "
           << "myapp: "
           << message << std::endl;
        return ss;
    }

private:
    std::string m_hostname;

    std::string getHostname() {
        char buf[256];
        gethostname(buf, sizeof(buf));
        return std::string(buf);
    }

    int getSyslogPriority() {
        // LOG_USER facility (8) + severity
        // NOTE: m_level is private in StreamLog and cannot be accessed here.
        // This method would require m_level to be made protected, or the
        // level would need to be passed in as a parameter.
        int severity = 6;  // default to info
        return (8 << 3) | severity;  // facility * 8 + severity
    }
};
```

### Structured Format with Context

```cpp
class ContextLogger : public StreamLog {
public:
    ContextLogger(const std::string& file, bool console = true)
        : StreamLog(file, console) {}

    void setContext(const std::string& key, const std::string& value) {
        m_context[key] = value;
    }

    void clearContext() {
        m_context.clear();
    }

    std::stringstream buildLog(const std::string& message) override {
        std::stringstream ss;
        // NOTE: levelToString() and m_level are private in StreamLog and are
        // not accessible here. The level label is omitted; to include it,
        // levelToString() would need to be made protected in the base class.
        ss << getTimestamp();

        for (const auto& pair : m_context) {
            ss << " " << pair.first << "=" << pair.second;
        }

        ss << " " << message << std::endl;
        return ss;
    }

private:
    std::map<std::string, std::string> m_context;
};

// Usage:
ContextLogger logger("app.log", true);
logger.setContext("request_id", "abc-123");
logger.setContext("user", "alice");
// Output: 1732835400 [INFO] request_id=abc-123 user=alice Processing request
```

## Using Custom Loggers

Once you've created a custom logger class, you can use it in several ways depending on your application's needs. The simplest approach is direct instantiation, which works well for applications that control their own initialization. For libraries or applications that want to use the global `log()` function with a custom logger, additional integration steps are needed. The following patterns cover the most common integration scenarios.

### Direct Instantiation

```cpp
// Create your custom logger
ISOLogger logger("app.log", true);

// Use via instance
logger.getLogStatement(INFO) << "Message";
```

### Replacing the Global Logger

To use a custom logger with the global `log()` function, you'll need to modify how the singleton is created. One approach is a factory function:

```cpp
// In your application
StreamLog& getLogger() {
    static ISOLogger instance("app.log", true);
    return instance;
}

// Modify log() to use your factory (requires code changes)
```

### Wrapper Macro

Create a macro that uses your custom logger:

```cpp
#define mylog(level) MyCustomLogger::instance().getLogStatement(level)

// Usage
mylog(INFO) << "Using custom logger";
```

## Adding New Functionality

Beyond customizing timestamps and formats, you can add entirely new functionality by extending the base class further. The inheritance model allows you to add new methods, member variables, and override additional protected methods. Common extensions include log rotation to manage file sizes, conditional coloring based on terminal detection, and buffering strategies for high-throughput logging. The following examples demonstrate more advanced customizations.

### Log Rotation

**Note:** `writeLog()` is private in the current `StreamLog` implementation and cannot be overridden in a derived class. The example below illustrates the intended design, but it will not compile without first changing `writeLog()` to `virtual` and `protected` in the base class.

```cpp
// DOES NOT COMPILE as-is: writeLog() is private in StreamLog.
// To enable this pattern, writeLog() must be declared virtual and protected
// in the base class.
class RotatingLogger : public StreamLog {
public:
    RotatingLogger(const std::string& basePath, size_t maxSize, int maxFiles)
        : StreamLog(basePath, true),
          m_basePath(basePath),
          m_maxSize(maxSize),
          m_maxFiles(maxFiles),
          m_currentSize(0) {}

private:
    void checkRotation() {
        if (m_currentSize >= m_maxSize) {
            rotate();
            m_currentSize = 0;
        }
    }

    void rotate() {
        // Rename existing files: .log.2 -> .log.3, .log.1 -> .log.2, etc.
        for (int i = m_maxFiles - 1; i > 0; --i) {
            std::string oldName = m_basePath + "." + std::to_string(i);
            std::string newName = m_basePath + "." + std::to_string(i + 1);
            std::rename(oldName.c_str(), newName.c_str());
        }
        std::rename(m_basePath.c_str(), (m_basePath + ".1").c_str());
    }

    std::string m_basePath;
    size_t m_maxSize;
    int m_maxFiles;
    size_t m_currentSize;
};
```

### Conditional Console Colors

```cpp
class SmartColorLogger : public StreamLog {
public:
    SmartColorLogger(const std::string& file)
        : StreamLog(file, isatty(STDERR_FILENO)) {}
};
```

## Best Practices

When extending StreamLog, follow these guidelines to maintain compatibility and avoid common pitfalls. Good extension design keeps customizations focused and minimal, overriding only what's necessary. Extensions should maintain the thread safety properties of the base class and be thoroughly tested to ensure they don't break existing functionality. The following guidelines summarize key considerations for robust custom loggers.

1. **Keep it simple**: Only override what you need
2. **Call parent methods**: When extending, consider calling `StreamLog::method()` for default behavior
3. **Thread safety**: Custom loggers should maintain thread safety if used in multi-threaded code
4. **Test thoroughly**: Custom formatters can break log parsing tools
5. **Document your format**: If using custom formats, document them for log analysis
