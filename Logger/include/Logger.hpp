// Logger.h
#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <mutex>
#include "CommonDef/ExportDef.h"

namespace spdlog
{
namespace sinks
{
class sink;
}
using sink_ptr = std::shared_ptr<sinks::sink>;
class logger;
namespace level
{
enum level_enum : int;
}
}  // namespace spdlog

class LIB_API Logger
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

    static Logger& getInstance();
    static void deleteInstance();

    void init(const std::string& loggerName = "console", LogLevel level = INFO_L, bool enableConsole = true, bool isSync = false,
              const std::string& filePath = "", size_t maxFileSize = 1048576 * 5, size_t maxFiles = 3);

    class LogStream;

    LogStream log(LogLevel eLevel, const char* file, int line, const char* func);
    LogStream trace(const char* file, int line, const char* func);
    LogStream debug(const char* file, int line, const char* func);
    LogStream info(const char* file, int line, const char* func);
    LogStream warn(const char* file, int line, const char* func);
    LogStream error(const char* file, int line, const char* func);
    LogStream critical(const char* file, int line, const char* func);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    void initSync(std::vector<spdlog::sink_ptr>, const std::string&, LogLevel);
    void initAsync(std::vector<spdlog::sink_ptr>, const std::string&, LogLevel);

private:
    Logger();
    ~Logger();

    struct LoggerImpl;
    std::unique_ptr<LoggerImpl> pimpl_;
    static inline Logger* m_pInstance = nullptr;
    static inline std::mutex m_mtxCreate;

    struct LogStreamImpl;
    friend class LogStream;
};

class LIB_API Logger::LogStream
{
public:
    LogStream(LogLevel level, LoggerImpl* loggerImpl, const char* file = nullptr, int line = 0,
              const char* func = nullptr);

    ~LogStream();

    template<typename T>
    LogStream& operator<<(const T& val);

    LogStream& operator<<(std::ostream& (*manip)(std::ostream&));

    LogStream& operator<<(const char* str);

private:
    std::unique_ptr<LogStreamImpl> pimpl_;
};

#define INIT_LOGGER(...) Logger::getInstance().init(__VA_ARGS__)
#define DELETE_LOGGER(...) Logger::deleteInstance()

#define LOG_TRACE Logger::getInstance().trace(__FILE__, __LINE__, __FUNCTION__)
#define LOG_DEBUG Logger::getInstance().debug(__FILE__, __LINE__, __FUNCTION__)
#define LOG_INFO Logger::getInstance().info(__FILE__, __LINE__, __FUNCTION__)
#define LOG_WARN Logger::getInstance().warn(__FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR Logger::getInstance().error(__FILE__, __LINE__, __FUNCTION__)
#define LOG_CRITICAL Logger::getInstance().critical(__FILE__, __LINE__, __FUNCTION__)
