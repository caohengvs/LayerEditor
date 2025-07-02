// MyLogger.h
#pragma once

#include <memory>
#include <ostream>
#include <string>
#include "CommonDef/ExportDef.h"

namespace spdlog
{
class logger;
namespace level
{
enum level_enum : int;
}
}  // namespace spdlog

class LIB_API MyLogger
{
public:
    enum LogLevel
    {
        TRACE_L = 0,
        DEBUG_L,
        INFO_L,
        WARN_L,
        ERROR_L,
        CRITICAL_L,
        OFF_L
    };

    static MyLogger& getInstance();

    void init(const std::string& loggerName = "console", LogLevel level = INFO_L, bool enableConsole = true,
              const std::string& filePath = "", size_t maxFileSize = 1048576 * 5, size_t maxFiles = 3);

    class LogStream;

    LogStream log(LogLevel eLevel,const char* file, int line, const char* func);
    LogStream trace(const char* file, int line, const char* func);
    LogStream debug(const char* file, int line, const char* func);
    LogStream info(const char* file, int line, const char* func);
    LogStream warn(const char* file, int line, const char* func);
    LogStream error(const char* file, int line, const char* func);
    LogStream critical(const char* file, int line, const char* func);

    MyLogger(const MyLogger&) = delete;
    MyLogger& operator=(const MyLogger&) = delete;

private:
    MyLogger();
    ~MyLogger();

    struct MyLoggerImpl;
    std::unique_ptr<MyLoggerImpl> pimpl_;

    struct LogStreamImpl;
    friend class LogStream;
};

class LIB_API MyLogger::LogStream
{
public:
    LogStream(LogLevel level, MyLoggerImpl* loggerImpl, const char* file = nullptr, int line = 0,
              const char* func = nullptr);

    ~LogStream();

    template<typename T>
    LogStream& operator<<(const T& val);

    LogStream& operator<<(std::ostream& (*manip)(std::ostream&));

    LogStream& operator<<(const char* str);

private:
    std::unique_ptr<LogStreamImpl> pimpl_;
};

#define INIT_MY_LOGGER(...) MyLogger::getInstance().init(__VA_ARGS__)

#define LOG_TRACE MyLogger::getInstance().trace(__FILE__, __LINE__, __FUNCTION__)
#define LOG_DEBUG MyLogger::getInstance().debug(__FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO MyLogger::getInstance().info(__FILE__, __LINE__, __FUNCTION__)
#define LOG_WARN MyLogger::getInstance().warn(__FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR MyLogger::getInstance().error(__FILE__, __LINE__, __FUNCTION__)
#define LOG_CRITICAL MyLogger::getInstance().critical(__FILE__, __LINE__, __FUNCTION__)
