#include "streamlog.hpp"
#include <iostream>

/**
 * Custom logger demonstrating inheritance and override capabilities.
 *
 * This example shows how to create a custom logger by inheriting from StreamLog
 * and overriding virtual methods to customize timestamp and log formatting.
 *
 * Note: This creates a direct instance (not a singleton). The base StreamLog
 * singleton is still available via log() for general use.
 */
class CustomLogger : public StreamLog {
public:
    CustomLogger() : StreamLog("custom_output.log", true) {}
    ~CustomLogger() {}

    // Rule of Five - delete copy and move operations
    CustomLogger(const CustomLogger&) = delete;
    CustomLogger(CustomLogger&&) = delete;
    CustomLogger& operator=(const CustomLogger&) = delete;
    CustomLogger& operator=(CustomLogger&&) = delete;

    /**
     * Override timestamp to use a custom format instead of Unix epoch.
     */
    std::string getTimestamp() const override {
        return "[NOW]";
    }

    /**
     * Override log formatting to use custom decorators.
     */
    std::stringstream buildLog(const std::string& message) const override {
        std::stringstream decorated_stream;
        decorated_stream << getTimestamp() << " ";
        decorated_stream << getColor();
        decorated_stream << "{{" << levelToString(m_level) << "}} ";
        decorated_stream << StreamColor::reset;
        decorated_stream << message << std::endl;
        return decorated_stream;
    }
};

int main() {
    // Create a custom logger instance (not using singleton pattern)
    CustomLogger custom_logger;

    // Use the custom logger
    custom_logger.getLogStatement(ERROR) << "ERROR FROM CUSTOM LOGGER";
    custom_logger.getLogStatement(INFO) << "Info message with custom formatting";

    // The base singleton is still available for general logging
    log(WARN) << "This uses the default singleton logger";

    return 0;
}
