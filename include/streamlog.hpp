#ifndef STREAMLOG_HPP
#define STREAMLOG_HPP

/**
 * @file streamlog.hpp
 * @brief A lightweight C++11 logging library with stream-based API and colored output
 * @author Derrek Landauer
 * @version 1.0.0
 *
 * StreamLog provides a simple, header-friendly logging interface with:
 * - Stream-based API familiar to C++ developers
 * - Six log levels with colored console output
 * - Compile-time log level filtering
 * - Zero external dependencies (C++11 standard library only)
 * - Optional STL container logging (vectors, maps)
 * - Thread-safe Meyer's singleton pattern
 * - Extensible via inheritance
 *
 * @section log_levels Log Levels
 *
 * - **TRACE**: Detailed debugging (function calls, variable dumps, memory addresses)
 * - **DEBUG**: Debug information helpful during development
 * - **INFO**: Normal operational messages, high-level execution tracking
 * - **WARN**: Warning conditions that may require attention
 * - **ERROR**: Error conditions requiring immediate attention
 * - **FATAL**: Critical failures causing application termination
 *
 * @section usage Basic Usage
 *
 * @code{.cpp}
 * #include <streamlog.hpp>
 *
 * int main() {
 *     log(INFO) << "Server started on port " << 8080;
 *     log(ERROR) << "Connection failed";
 *     return 0;
 * }
 * @endcode
 *
 * @section config Configuration
 *
 * Customize output file and console logging:
 * @code{.cpp}
 * StreamLog::instance("myapp.log", true);  // custom file, enable console
 * @endcode
 *
 * Set compile-time log level filtering:
 * @code{.bash}
 * make DEBUG_LEVEL=3  # Only INFO and above
 * @endcode
 */

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef ENABLE_VECTOR_LOGGING
#include <vector>
#endif

#ifdef ENABLE_MAP_LOGGING
#include <map>
#endif

/**
 * @brief Log severity levels
 *
 * Lower levels encompass higher levels when filtering is applied.
 */
enum LogLevel {
  TRACE, ///< Detailed debugging information
  DEBUG, ///< Debug information
  INFO,  ///< Informational messages
  WARN,  ///< Warning conditions
  ERROR, ///< Error conditions
  FATAL  ///< Critical failures
};

#if DEBUG_LEVEL == 1
#define LOG_LEVEL TRACE
#elif DEBUG_LEVEL == 2
#define LOG_LEVEL DEBUG
#elif DEBUG_LEVEL == 3
#define LOG_LEVEL INFO
#elif DEBUG_LEVEL == 4
#define LOG_LEVEL WARN
#elif DEBUG_LEVEL == 5
#define LOG_LEVEL ERROR
#elif DEBUG_LEVEL == 6
#define LOG_LEVEL FATAL
#endif

/**
 * @brief ANSI color codes for log levels
 *
 * Color definitions are loaded from theme headers at compile time.
 * Default theme uses standard ANSI colors.
 */
struct StreamColor {
  static const std::string TraceColor; ///< TRACE level color
  static const std::string DebugColor; ///< DEBUG level color
  static const std::string InfoColor;  ///< INFO level color
  static const std::string WarnColor;  ///< WARN level color
  static const std::string ErrorColor; ///< ERROR level color
  static const std::string FatalColor; ///< FATAL level color
  static const std::string reset;      ///< Reset to default color
};

/**
 * @brief Main logging class with singleton pattern
 *
 * StreamLog provides a thread-safe singleton logger that writes to both
 * file and optionally console. Uses RAII pattern via LogStatement to
 * ensure messages are committed when the statement goes out of scope.
 *
 * @note This class uses Meyer's singleton (C++11 thread-safe static local).
 *       Copy and move operations are explicitly deleted.
 */
class StreamLog {
public:
  /**
   * @brief RAII wrapper for building and committing log messages
   *
   * LogStatement accumulates message fragments via operator<< and commits
   * the complete message when destroyed. Supports streaming of built-in types,
   * strings, and optionally STL containers.
   */
  class LogStatement {
  public:
    /**
     * @brief Move constructor
     * @param other Source LogStatement to move from
     */
    LogStatement(LogStatement &&other) noexcept;

    /**
     * @brief Construct LogStatement bound to a logger
     * @param logger Reference to parent StreamLog instance
     */
    LogStatement(StreamLog &logger);

    /**
     * @brief Stream insertion operator for generic types
     * @tparam T Type supporting operator<< to std::ostream
     * @param value Value to append to log message
     * @return Reference to this LogStatement for chaining
     */
    template <typename T> LogStatement &operator<<(const T &value) {
      m_buffer << value;
      return *this;
    }

    /**
     * @brief Stream insertion operator for C-strings
     * @param value C-string to append
     * @return Reference to this LogStatement for chaining
     */
    LogStatement &operator<<(const char *value) {
      m_buffer << value;
      return *this;
    }

#ifdef ENABLE_VECTOR_LOGGING
    /**
     * @brief Stream insertion operator for std::vector
     * @tparam T Vector element type
     * @param vec Vector to format and append
     * @return Reference to this LogStatement for chaining
     * @note Only available if compiled with -DENABLE_VECTOR_LOGGING
     */
    template <typename T> LogStatement &operator<<(const std::vector<T> &vec) {
      m_buffer << "[";
      for (size_t i = 0; i < vec.size(); ++i) {
        m_buffer << vec[i];
        if (i != vec.size() - 1) {
          m_buffer << ", ";
        }
      }
      m_buffer << "]";
      return *this;
    }
#endif

#ifdef ENABLE_MAP_LOGGING
    /**
     * @brief Stream insertion operator for std::map
     * @tparam K Map key type
     * @tparam V Map value type
     * @param m Map to format and append
     * @return Reference to this LogStatement for chaining
     * @note Only available if compiled with -DENABLE_MAP_LOGGING
     */
    template <typename K, typename V>
    LogStatement &operator<<(const std::map<K, V> &m) {
      m_buffer << "{";
      for (auto it = m.begin(); it != m.end(); ++it) {
        m_buffer << it->first << ": " << it->second;
        if (std::next(it) != m.end()) {
          m_buffer << ", ";
        }
      }
      m_buffer << "}";
      return *this;
    }
#endif
    /**
     * @brief Clear the internal message buffer
     */
    void clearBuffer();

    /**
     * @brief Get current buffer contents
     * @return String representation of accumulated message
     */
    std::string getBufferContent() const;

    /**
     * @brief Append content to buffer
     * @param content String to append
     */
    void appendToBuffer(const std::string &content);

    /**
     * @brief Destructor commits the log message
     *
     * RAII pattern: message is written to file/console when LogStatement
     * goes out of scope.
     */
    ~LogStatement();

  private:
    StreamLog &m_logger;
    std::ostringstream m_buffer;
  };

  /**
   * @brief Create a LogStatement for the given log level
   * @param level Log severity level
   * @return LogStatement ready for message streaming
   */
  LogStatement getLogStatement(LogLevel level);

  /**
   * @brief Stream manipulator support (e.g., std::endl)
   * @param manipulator Stream manipulator function
   * @return LogStatement for chaining
   */
  LogStatement operator<<(std::ostream &(*manipulator)(std::ostream &));

public:
  /**
   * @brief Get the singleton logger instance
   *
   * Uses Meyer's singleton pattern (thread-safe in C++11+).
   *
   * @param fileName Log file path (only used on first call)
   * @param consoleOutput Enable console output (only used on first call)
   * @return Reference to singleton StreamLog instance
   *
   * @note Subsequent calls ignore fileName and consoleOutput parameters.
   *       The instance retains configuration from the first call.
   *
   * @warning Do not call with different parameters in the same program.
   */
  static StreamLog &instance(const std::string &fileName = "output.log",
                             bool consoleOutput = false);

  /**
   * @brief Virtual destructor for inheritance support
   */
  virtual ~StreamLog();

  // Delete copy and move operations (Rule of Five)
  StreamLog(const StreamLog &) = delete; ///< No copy constructor
  StreamLog &operator=(const StreamLog &) = delete; ///< No copy assignment
  StreamLog(StreamLog &&) = delete; ///< No move constructor
  StreamLog &operator=(StreamLog &&) = delete; ///< No move assignment

protected:
  /**
   * @brief Construct logger with file and console settings
   * @param fileName Path to log file
   * @param consoleOutput true to also write to stderr
   *
   * @note Protected to allow inheritance. Use instance() for normal usage.
   */
  explicit StreamLog(const std::string &fileName, bool consoleOutput = false);

private:
  LogLevel m_level;       ///< Current log level being written
  LogLevel m_threshold;   ///< Minimum level to actually write (compile-time)

  std::string m_fileName;  ///< Path to log file
  bool m_consoleOutput;    ///< Whether to also write to stderr

  /**
   * @brief Convert LogLevel enum to string
   * @param level Log level to convert
   * @return String representation ("TRACE", "DEBUG", etc.)
   */
  std::string levelToString(const LogLevel &level) const;

  /**
   * @brief Get ANSI color code for current log level
   * @return Color code string
   */
  std::string getColor() const;

  /**
   * @brief Get current timestamp
   * @return Unix timestamp as string (seconds since epoch)
   * @note Virtual to allow custom timestamp formatting via inheritance
   */
  virtual std::string getTimestamp() const;

  /**
   * @brief Build formatted log message with timestamp and level
   * @param message Raw message content
   * @return Formatted log line with timestamp, color, and level
   * @note Virtual to allow custom formatting via inheritance
   */
  virtual std::stringstream buildLog(const std::string &message) const;

  /**
   * @brief Write formatted message to file and optionally console
   * @param message Message to write
   */
  void writeLog(const std::string &message);

  /**
   * @brief Commit message if it meets threshold
   * @param message Message to commit
   */
  void commitLog(const std::string &message);

  /**
   * @brief Create directories recursively for log file path
   * @param path Directory path to create
   * @return true if successful or directory exists, false on error
   */
  bool createDirectories(const std::string &path) const;
};

/**
 * @brief Global logging function
 * @param level Log severity level
 * @return LogStatement ready for message streaming
 *
 * @code{.cpp}
 * log(INFO) << "Server started on port " << 8080;
 * @endcode
 */
StreamLog::LogStatement log(LogLevel level);
#endif
