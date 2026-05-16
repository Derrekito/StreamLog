#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

/**
 * @file test_framework.hpp
 * @brief Minimal C++11 test framework for StreamLog
 */

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace TestFramework {

struct TestResult {
  std::string name;
  bool passed;
  std::string message;
};

class TestSuite {
public:
  static TestSuite &instance() {
    static TestSuite suite;
    return suite;
  }

  void addTest(const std::string &name, std::function<void()> testFunc) {
    tests.push_back({name, testFunc});
  }

  int run() {
    int passed = 0;
    int failed = 0;

    std::cout << "\n=== Running Tests ===\n" << std::endl;

    for (const auto &test : tests) {
      try {
        test.func();
        std::cout << "✓ " << test.name << std::endl;
        passed++;
      } catch (const std::exception &e) {
        std::cout << "✗ " << test.name << "\n  " << e.what() << std::endl;
        failed++;
      }
    }

    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Total:  " << (passed + failed) << std::endl;

    return failed;
  }

private:
  struct Test {
    std::string name;
    std::function<void()> func;
  };

  std::vector<Test> tests;
};

class AssertionError : public std::runtime_error {
public:
  explicit AssertionError(const std::string &msg) : std::runtime_error(msg) {}
};

inline void assertTrue(bool condition, const std::string &message) {
  if (!condition) {
    throw AssertionError("Assertion failed: " + message);
  }
}

inline void assertFalse(bool condition, const std::string &message) {
  if (condition) {
    throw AssertionError("Assertion failed: " + message);
  }
}

inline void assertEquals(const std::string &expected, const std::string &actual,
                         const std::string &message = "") {
  if (expected != actual) {
    throw AssertionError("Assertion failed: " + message + "\n  Expected: '" +
                         expected + "'\n  Actual:   '" + actual + "'");
  }
}

template <typename T>
inline void assertEquals(T expected, T actual,
                         const std::string &message = "") {
  if (expected != actual) {
    throw AssertionError("Assertion failed: " + message);
  }
}

} // namespace TestFramework

#define TEST(name)                                                             \
  void test_##name();                                                          \
  namespace {                                                                  \
  struct TestRegistrar_##name {                                                \
    TestRegistrar_##name() {                                                   \
      TestFramework::TestSuite::instance().addTest(#name, test_##name);        \
    }                                                                          \
  };                                                                           \
  static TestRegistrar_##name registrar_##name;                                \
  }                                                                            \
  void test_##name()

#endif // TEST_FRAMEWORK_HPP
