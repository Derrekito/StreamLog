#ifndef STREAMLOG_HPP
#define STREAMLOG_HPP

/*
 * The StreamLog class provides an interface for logging messages to a file
 * and/or console, with the ability to specify the logging level and
 * color-coding of the messages. The class supports the following logging levels
 * TRACE, DEBUG, INFO, WARN, ERROR, and FATAL.
 *
 * Trace-level debugging is a method used to see detailed information about the
 * execution of a code, such as function calls, variable values, and memory
 * addresses, to help identify problems in complex systems.
 *
 * Debug level logging is used to record information that is helpful for
 * debugging an application. It is generally less detailed than trace-level
 * logging but provides a sufficient level of information for developers to
 * identify and fix problems in the code.
 *
 * Info-level logging is used to record information about the regular operation
 * of an application. It is typically less detailed than debug-level logging and
 * is used to track the progress and performance of the application. It provides
 * a high-level view of the application's execution. It can be used to track the
 * performance of critical processes, identify potential bottlenecks, and
 * monitor the application's overall health.
 *
 * Warning-level logging records information about potential issues or problems
 * in an application. It is typically less detailed than debug-level logging but
 * provides enough information to indicate that something may be wrong and
 * requires attention. Warning-level logging alerts developers to potential
 * issues that may not be causing immediate problems but could lead to issues in
 * the future.
 *
 * Error-level logging records information about errors or exceptions that occur
 * in an application. It provides enough information to indicate that something
 * has gone wrong and that the application may not be able to continue regular
 * operation. Error-level logging alerts developers to problems that need
 * immediate attention.
 *
 * Fatal-level logging records information about critical errors or unhandled
 * exceptions that cause the application to stop running. Fatal level logging is
 * the most severe type and is used to alert developers to critical issues that
 * require immediate attention.
 *
 * The main difference between error and fatal level logging is that Error level
 * logging is used to notify errors that may allow the application to continue
 * regular operation but need attention. In contrast, Fatal level logging is
 * used to notify critical errors that cause the application to stop running and
 * need immediate attention.
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

enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

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

bool create_recursive(const std::string &path);

struct StreamColor {
  static const std::string TraceColor;
  static const std::string DebugColor;
  static const std::string InfoColor;
  static const std::string WarnColor;
  static const std::string ErrorColor;
  static const std::string FatalColor;
  static const std::string reset;
};

class StreamLog {
public:
  class LogStatement {
  public:
    LogStatement(LogStatement &&other) noexcept;

    LogStatement(StreamLog &logger);

    template <typename T> LogStatement &operator<<(const T &value) {
      m_buffer << value;
      return *this;
    }

    LogStatement &operator<<(const char *value) {
      m_buffer << value;
      return *this;
    }

#ifdef ENABLE_VECTOR_LOGGING
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
    template <typename K, typename V>
    LogStatement &operator<<(const std::map<K, V> &m) {
      m_buffer << "{";
      for (auto it = m.begin(); it != m.end(); ++it) {
        m_buffer << it->first << ": " << it->second;
        if (std::next(it) != m.end()) { // Check if this is the last iteration
          m_buffer << ", ";
        }
      }
      m_buffer << "}";
      return *this;
    }
#endif
    void clearBuffer();
    std::string getBufferContent() const;
    void appendToBuffer(const std::string &content);

    ~LogStatement();

  private:
    StreamLog &m_logger;
    std::ostringstream m_buffer;
  };

  LogStatement getLogStatement(LogLevel level);

  LogStatement operator<<(std::ostream &(*manipulator)(std::ostream &));

public:
  // Method to get the singleton instance of StreamLog
  // NOTE: fileName and consoleOutput are only used on the FIRST call.
  // Subsequent calls return the same instance with original parameters.
  static StreamLog &instance(const std::string &fileName = "output.log",
                             bool consoleOutput = false);

  virtual ~StreamLog();

  // Delete copy and move operations (Rule of Five)
  StreamLog(const StreamLog &) = delete;
  StreamLog &operator=(const StreamLog &) = delete;
  StreamLog(StreamLog &&) = delete;
  StreamLog &operator=(StreamLog &&) = delete;

protected:
  // Constructor takes in the file name to write logs to and a boolean
  // indicating whether or not to also output to console
  explicit StreamLog(const std::string &fileName, bool consoleOutput = false);

private:
  LogLevel m_level;
  LogLevel m_threshold;

  std::string m_fileName;
  bool m_consoleOutput;

  std::string levelToString(const LogLevel &level) const;
  std::string getColor() const;
  virtual std::string getTimestamp() const;
  virtual std::stringstream buildLog(const std::string &message) const;
  void writeLog(const std::string &message);
  void commitLog(const std::string &message);
};

StreamLog::LogStatement log(LogLevel level);
#endif
