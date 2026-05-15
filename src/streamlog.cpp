#include <streamlog.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <string>
#include <cstring>
#include <iostream>

#define LOG_FILE "output.log"

bool StreamLog::createDirectories(const std::string& path) const
{
    size_t pos = 0;
    while ((pos = path.find('/', pos + 1)) != std::string::npos)
    {
        std::string dir = path.substr(0, pos);
        if (mkdir(dir.c_str(), 0777) == -1)
        {
            if (errno != EEXIST)
            {
                return false;
            }
        }
    }
    return true;
}

// Theme selection
#if defined(STREAMLOG_THEME_ROSE_PINE_MOON)
#include "themes/rose_pine_moon.hpp"
#else
#include "themes/default.hpp"
#endif

// Initialize the color constants
const std::string StreamColor::TraceColor = STREAMLOG_COLOR_TRACE;
const std::string StreamColor::DebugColor = STREAMLOG_COLOR_DEBUG;
const std::string StreamColor::InfoColor  = STREAMLOG_COLOR_INFO;
const std::string StreamColor::WarnColor  = STREAMLOG_COLOR_WARN;
const std::string StreamColor::ErrorColor = STREAMLOG_COLOR_ERROR;
const std::string StreamColor::FatalColor = STREAMLOG_COLOR_FATAL;
const std::string StreamColor::reset      = STREAMLOG_COLOR_RESET;

// Constructor for the StreamLog class, sets the file name and whether to output to console
StreamLog::StreamLog(const std::string& fileName,
               bool consoleOutput) : m_threshold(LOG_LEVEL),
                                     m_fileName(fileName),
                                     m_consoleOutput(consoleOutput)
{}

// Constructor for the LogStatement subclass, sets the log level
StreamLog::LogStatement::LogStatement(StreamLog& logger) : m_logger(logger) {}

StreamLog::LogStatement::LogStatement(LogStatement&& other) noexcept
    : m_logger(other.m_logger), m_buffer(other.m_buffer.str())
{
    other.m_buffer.str("");  // Clear the source buffer
}

// Destructor for the StreamLog class
StreamLog::~StreamLog()
{
    // Singleton cleanup happens automatically at program exit
    // Do NOT delete m_instance here - that would be deleting 'this'!
}

// Method to get the singleton instance of StreamLog
// Meyer's singleton: thread-safe in C++11+, no manual memory management
StreamLog& StreamLog::instance(const std::string& fileName, bool consoleOutput)
{
    static StreamLog instance(fileName, consoleOutput);
    return instance;
}

// Converts log level to string representation
std::string StreamLog::levelToString(const LogLevel& level) const
{
    switch(level)
    {
        case TRACE: return "TRACE";
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default: return "";
    }
}

// Returns the color code based on log level
std::string StreamLog::getColor() const
{
    switch(m_level)
    {
        case TRACE: return StreamColor::TraceColor;
        case DEBUG: return StreamColor::DebugColor;
        case INFO: return StreamColor::InfoColor;
        case WARN: return StreamColor::WarnColor;
        case ERROR: return StreamColor::ErrorColor;
        case FATAL: return StreamColor::FatalColor;
        default: return "";
    }
}

std::string StreamLog::getTimestamp() const
{
    // Store the current time in seconds (since the UNIX epoch)
    time_t now = std::time(NULL);

    std::stringstream ss;
    ss << now;

    // Convert 'now' to a string and return it
    return ss.str();
}

std::stringstream StreamLog::buildLog(const std::string& message) const
{
    std::stringstream decorated_stream;
    decorated_stream << getTimestamp() << " ";
    decorated_stream << getColor();
    decorated_stream << "[" << levelToString(m_level) << "] ";
    decorated_stream << StreamColor::reset; // Assuming this returns a string or character sequence
    decorated_stream << message << std::endl;
    return decorated_stream;
}

void StreamLog::writeLog(const std::string& message)
{
    if (message.empty())
    {
        return;
    }

    // Before opening the log file:
    if (!createDirectories(m_fileName))
    {
        std::cerr << "Failed to create directory for log file: " << m_fileName << std::endl;
        // Don't log this message, but don't crash the application either
        return;
    }

    // open file in append mode
    std::ofstream out(m_fileName.c_str(), std::ios_base::app);

    // sanity check
    if (!out.is_open())
    {
        std::cerr << "Error: Failed to open log file" << std::endl;
        return;
    }

    // Write the decorated message to the log file and console
    std::string decorated_msg = buildLog(message).str();
    if (m_consoleOutput)
    {
        std::cerr << decorated_msg;
    }
    out << decorated_msg;
    out.close();
}

void StreamLog::commitLog(const std::string& message)
{
    if (m_level >= m_threshold)
    {
        writeLog(message);
    }
}


StreamLog::LogStatement StreamLog::operator<<(std::ostream& (*manipulator)(std::ostream&))
{
    LogStatement stmt(*this);
    stmt << manipulator;
    return stmt;
}

void StreamLog::LogStatement::clearBuffer()
{
    m_buffer.str("");
    m_buffer.clear();
}

std::string StreamLog::LogStatement::getBufferContent() const
{
    return m_buffer.str();
}

void StreamLog::LogStatement::appendToBuffer(const std::string& content)
{
    m_buffer << content;
}

StreamLog::LogStatement StreamLog::getLogStatement(LogLevel level)
{
    m_level = level;
    return LogStatement(*this);
}

StreamLog::LogStatement::~LogStatement()
{
    m_logger.commitLog(m_buffer.str());
}

StreamLog::LogStatement log(LogLevel level)
{
    // Get the Singleton instance of StreamLog
    static StreamLog& logger = StreamLog::instance(LOG_FILE, true);
    return logger.getLogStatement(level);
}
