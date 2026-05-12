#include "streamlog.hpp"
#include <iostream>

#define LOG_FILE "output.log"

class LoggerJR : public StreamLog {
public:
    LoggerJR() : StreamLog("temp_log.txt", true) {}
    ~LoggerJR() {}
    LoggerJR(const LoggerJR& ljr) = delete;
    LoggerJR(LoggerJR&& ljr) = delete;
    LoggerJR& operator=(const LoggerJR& ljr) = delete;
    LoggerJR& operator=(LoggerJR&& ljr) = delete;

    std::string getTimestamp() const override {
        return "NOW";
    }

    std::stringstream buildLog(const std::string& message) const override {
        std::stringstream decorated_stream;
        decorated_stream << getTimestamp() << " ";
        decorated_stream << "{{" << "TEST LEVEL" << "}} ";
        decorated_stream << message << std::endl;
        return decorated_stream;
    }

    StreamLog::LogStatement operator()(LogLevel level) {
        return StreamLog::instance(LOG_FILE, true).getLogStatement(level);
    }
};

int main() {
    LoggerJR logger_jr;

    logger_jr(ERROR) << "ERROR FROM JR LOGGER";

    return 0;
}
