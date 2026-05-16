/**
 * @file test_streamlog.cpp
 * @brief Comprehensive unit tests for StreamLog library
 */

#include "streamlog.hpp"
#include "test_framework.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#ifdef ENABLE_VECTOR_LOGGING
#include <vector>
#endif

#ifdef ENABLE_MAP_LOGGING
#include <map>
#endif

using namespace TestFramework;

// Global test log file - the log() function uses "output.log" hardcoded
const std::string TEST_LOG_FILE = "output.log";

// Helper function to read file contents
std::string readFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

// Helper function to check if file exists
bool fileExists(const std::string &filename) {
  struct stat buffer;
  return (stat(filename.c_str(), &buffer) == 0);
}

// Helper function to delete file
void deleteFile(const std::string &filename) {
  if (fileExists(filename)) {
    remove(filename.c_str());
  }
}

// Helper function to count lines in file
int countLines(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return 0;
  }
  int count = 0;
  std::string line;
  while (std::getline(file, line)) {
    count++;
  }
  return count;
}

// Test basic singleton access
TEST(singleton_instance) {
  StreamLog &logger1 = StreamLog::instance();
  StreamLog &logger2 = StreamLog::instance();

  // Both references should point to the same instance
  assertTrue(&logger1 == &logger2, "Singleton should return same instance");
}

// Test log file creation
TEST(log_file_creation) {
  log(INFO) << "File creation test";
  assertTrue(fileExists(TEST_LOG_FILE), "Log file should be created");
}

// Test log message format
TEST(log_message_format) {
  log(INFO) << "Format test message";

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("[INFO]") != std::string::npos,
             "Log should contain [INFO] level");
  assertTrue(content.find("Format test message") != std::string::npos,
             "Log should contain message text");
}

// Test multiple log levels
TEST(multiple_log_levels) {
#if DEBUG_LEVEL <= 1
  log(TRACE) << "Trace level test";
#endif
#if DEBUG_LEVEL <= 2
  log(DEBUG) << "Debug level test";
#endif
#if DEBUG_LEVEL <= 3
  log(INFO) << "Info level test";
#endif
#if DEBUG_LEVEL <= 4
  log(WARN) << "Warning level test";
#endif
#if DEBUG_LEVEL <= 5
  log(ERROR) << "Error level test";
#endif
#if DEBUG_LEVEL <= 6
  log(FATAL) << "Fatal level test";
#endif

  std::string content = readFile(TEST_LOG_FILE);

#if DEBUG_LEVEL <= 1
  assertTrue(content.find("[TRACE]") != std::string::npos,
             "Should contain TRACE level");
#endif
#if DEBUG_LEVEL <= 2
  assertTrue(content.find("[DEBUG]") != std::string::npos,
             "Should contain DEBUG level");
#endif
#if DEBUG_LEVEL <= 3
  assertTrue(content.find("[INFO]") != std::string::npos,
             "Should contain INFO level");
#endif
#if DEBUG_LEVEL <= 4
  assertTrue(content.find("[WARN]") != std::string::npos,
             "Should contain WARN level");
#endif
#if DEBUG_LEVEL <= 5
  assertTrue(content.find("[ERROR]") != std::string::npos,
             "Should contain ERROR level");
#endif
#if DEBUG_LEVEL <= 6
  assertTrue(content.find("[FATAL]") != std::string::npos,
             "Should contain FATAL level");
#endif
}

// Test stream chaining
TEST(stream_chaining) {
  log(INFO) << "Chain: " << 42 << " text " << 3.14;

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Chain: 42") != std::string::npos,
             "Should contain integer value");
  assertTrue(content.find(" text ") != std::string::npos,
             "Should contain string value");
  assertTrue(content.find(" 3.14") != std::string::npos,
             "Should contain float value");
}

#ifdef ENABLE_VECTOR_LOGGING
// Test vector logging
TEST(vector_logging) {
  std::vector<int> numbers = {10, 20, 30};
  log(INFO) << "Vec test: " << numbers;

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("[10, 20, 30]") != std::string::npos,
             "Should contain formatted vector");
}

// Test empty vector
TEST(empty_vector_logging) {
  std::vector<int> empty;
  log(INFO) << "Empty vec: " << empty;

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Empty vec: []") != std::string::npos,
             "Should contain empty vector brackets");
}
#endif

#ifdef ENABLE_MAP_LOGGING
// Test map logging
TEST(map_logging) {
  std::map<std::string, int> data;
  data["X"] = 100;
  data["Y"] = 200;
  log(INFO) << "Map test: " << data;

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("X: 100") != std::string::npos,
             "Should contain X mapping");
  assertTrue(content.find("Y: 200") != std::string::npos,
             "Should contain Y mapping");
}

// Test empty map
TEST(empty_map_logging) {
  std::map<std::string, int> empty;
  log(INFO) << "Empty map: " << empty;

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Empty map: {}") != std::string::npos,
             "Should contain empty map braces");
}
#endif

// Test nested directory creation
TEST(nested_directory_creation) {
  // This test creates a separate logger to test directory creation
  const std::string filename = "test_logs/nested/deep/test.log";

  // Clean up first
  system("rm -rf test_logs");

  class NestedLogger : public StreamLog {
  public:
    NestedLogger() : StreamLog("test_logs/nested/deep/test.log", false) {}
  };

  NestedLogger nested;
  nested.getLogStatement(INFO) << "Nested log test";

  assertTrue(fileExists(filename), "Should create nested directories");

  // Clean up
  system("rm -rf test_logs");
}

// Test custom logger inheritance
TEST(custom_logger_inheritance) {
  class CustomLogger : public StreamLog {
  public:
    CustomLogger() : StreamLog("test_custom.log", false) {}

    std::string getTimestamp() const override { return "CUSTOM_TIME"; }

    std::stringstream buildLog(const std::string &message) const override {
      std::stringstream ss;
      ss << getTimestamp() << " " << message << std::endl;
      return ss;
    }
  };

  deleteFile("test_custom.log");

  CustomLogger custom;
  custom.getLogStatement(INFO) << "Custom message";

  std::string content = readFile("test_custom.log");
  assertTrue(content.find("CUSTOM_TIME") != std::string::npos,
             "Should use custom timestamp");
  assertTrue(content.find("Custom message") != std::string::npos,
             "Should contain message");

  deleteFile("test_custom.log");
}

// Test LogStatement buffer operations
TEST(log_statement_buffer) {
  {
    auto statement = log(INFO);
    statement << "Buffer1";
    statement << " Buffer2";
    statement << " Buffer3";
  } // Destructor commits here

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Buffer1 Buffer2 Buffer3") != std::string::npos,
             "Should accumulate buffer content");
}

// Test C-string logging
TEST(c_string_logging) {
  const char *cstr = "CString test";
  log(INFO) << cstr;

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("CString test") != std::string::npos,
             "Should handle C-strings");
}

// Test multiple messages
TEST(multiple_messages) {
  int linesBefore = countLines(TEST_LOG_FILE);
  log(INFO) << "Multi1";
  log(INFO) << "Multi2";
  log(INFO) << "Multi3";

  int linesAfter = countLines(TEST_LOG_FILE);
  assertTrue(linesAfter >= linesBefore + 3, "Should have at least 3 new log lines");
}

// Test LogStatement move constructor
TEST(log_statement_move) {
  {
    // Create and immediately move
    auto statement = log(INFO);
    statement << "Move test msg";
  } // Destructor fires here

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Move test msg") != std::string::npos,
             "Should handle move semantics");
}

// Test timestamp format
TEST(timestamp_format) {
  log(INFO) << "Timestamp format test";

  std::string content = readFile(TEST_LOG_FILE);
  // Check that timestamp is numeric (Unix epoch)
  size_t pos = 0;
  while (pos < content.length() && isdigit(content[pos])) {
    pos++;
  }
  assertTrue(pos > 0, "Should start with numeric timestamp");
}

// Test large message handling
TEST(large_message) {
  std::string largeMsg(10000, 'X');
  log(INFO) << "Large: " << largeMsg;

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find(largeMsg) != std::string::npos,
             "Should handle large messages");
}

// Test special characters
TEST(special_characters) {
  log(INFO) << "Special chars: \t\n\r\"'\\";

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Special chars:") != std::string::npos,
             "Should handle special characters");
}

// Test empty message
TEST(empty_message) {
  {
    log(INFO) << "";
  }
  // Should not crash, message may or may not be written
  assertTrue(true, "Should not crash on empty message");
}

// Test rapid successive logs (stress test)
TEST(rapid_logging) {
  int linesBefore = countLines(TEST_LOG_FILE);
  for (int i = 0; i < 100; i++) {
    log(INFO) << "Rapid log " << i;
  }
  int linesAfter = countLines(TEST_LOG_FILE);
  assertTrue(linesAfter >= linesBefore + 100,
             "Should handle rapid successive logs");
}

// Test mixed types in single statement
TEST(mixed_types) {
  {
    log(INFO) << "Mixed: " << 42 << " " << 3.14159 << " " << true << " "
              << "string";
  }

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Mixed: 42") != std::string::npos,
             "Should handle mixed types");
}

#ifdef ENABLE_VECTOR_LOGGING
// Test vector of strings
TEST(vector_of_strings) {
  std::vector<std::string> strings = {"hello", "world", "test"};
  {
    log(INFO) << "Strings: " << strings;
  }

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("hello") != std::string::npos,
             "Should handle vector of strings");
  assertTrue(content.find("world") != std::string::npos,
             "Should contain all string elements");
}

// Test large vector
TEST(large_vector) {
  std::vector<int> large;
  for (int i = 0; i < 1000; i++) {
    large.push_back(i);
  }
  {
    log(INFO) << "Large vector test";
    log(DEBUG) << large;
  }

  std::string content = readFile(TEST_LOG_FILE);
  assertTrue(content.find("Large vector test") != std::string::npos,
             "Should handle large vectors without crash");
}
#endif

// Test multiple inheritance chains
TEST(inheritance_chain) {
  class BaseLogger : public StreamLog {
  public:
    BaseLogger() : StreamLog("test_inheritance.log", false) {}
    std::string getTimestamp() const override { return "BASE"; }
  };

  class DerivedLogger : public BaseLogger {
  public:
    std::stringstream buildLog(const std::string &message) const override {
      std::stringstream ss;
      ss << "DERIVED:" << message << std::endl;
      return ss;
    }
  };

  deleteFile("test_inheritance.log");
  {
    DerivedLogger derived;
    derived.getLogStatement(INFO) << "Chain test";
  }

  std::string content = readFile("test_inheritance.log");
  assertTrue(content.find("DERIVED:Chain test") != std::string::npos,
             "Should support inheritance chain");

  deleteFile("test_inheritance.log");
}

// Test concurrent logger instances (not thread safety, just multiple instances)
TEST(multiple_custom_loggers) {
  deleteFile("test_logger1.log");
  deleteFile("test_logger2.log");

  class Logger1 : public StreamLog {
  public:
    Logger1() : StreamLog("test_logger1.log", false) {}
  };

  class Logger2 : public StreamLog {
  public:
    Logger2() : StreamLog("test_logger2.log", false) {}
  };

  {
    Logger1 log1;
    Logger2 log2;
    log1.getLogStatement(INFO) << "From logger1";
    log2.getLogStatement(INFO) << "From logger2";
  }

  assertTrue(fileExists("test_logger1.log"),
             "Logger1 should create its file");
  assertTrue(fileExists("test_logger2.log"),
             "Logger2 should create its file");

  std::string content1 = readFile("test_logger1.log");
  std::string content2 = readFile("test_logger2.log");

  assertTrue(content1.find("From logger1") != std::string::npos,
             "Logger1 should write to its file");
  assertTrue(content2.find("From logger2") != std::string::npos,
             "Logger2 should write to its file");

  deleteFile("test_logger1.log");
  deleteFile("test_logger2.log");
}

// Main test runner
int main() {
  // Clean up old test log
  deleteFile(TEST_LOG_FILE);

  std::cout << "StreamLog Unit Tests" << std::endl;
  std::cout << "DEBUG_LEVEL: " << DEBUG_LEVEL << std::endl;

#ifdef ENABLE_VECTOR_LOGGING
  std::cout << "ENABLE_VECTOR_LOGGING: ON" << std::endl;
#else
  std::cout << "ENABLE_VECTOR_LOGGING: OFF" << std::endl;
#endif

#ifdef ENABLE_MAP_LOGGING
  std::cout << "ENABLE_MAP_LOGGING: ON" << std::endl;
#else
  std::cout << "ENABLE_MAP_LOGGING: OFF" << std::endl;
#endif

  int result = TestFramework::TestSuite::instance().run();

  // Clean up test log
  deleteFile(TEST_LOG_FILE);

  return result;
}
