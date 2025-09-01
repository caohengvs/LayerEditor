#include "Logger.hpp"
#include <spdlog/async.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <vector>


struct LoggerImpl
{
    std::shared_ptr<spdlog::logger> logger_;

    static spdlog::level::level_enum mapLogLevel(Logger::LogLevel level)
    {
        switch (level)
        {
            case Logger::TRACE_L:
                return spdlog::level::trace;
            case Logger::DEBUG_L:
                return spdlog::level::debug;
            case Logger::INFO_L:
                return spdlog::level::info;
            case Logger::WARN_L:
                return spdlog::level::warn;
            case Logger::ERROR_L:
                return spdlog::level::err;
            case Logger::CRITICAL_L:
                return spdlog::level::critical;
            case Logger::OFF_L:
                return spdlog::level::off;
            default:
                return spdlog::level::info;
        }
    }

    LoggerImpl() = default;

    ~LoggerImpl() = default;
};

Logger::Logger()
    : pimpl_(std::make_unique<LoggerImpl>())
{
}

Logger::~Logger() = default;

Logger& Logger::getInstance()
{
    if (m_pInstance)
        return *m_pInstance;

    std::lock_guard lock(m_mtxCreate);
    m_pInstance = new Logger;
    return *m_pInstance;
}

void Logger::deleteInstance()
{
    if (!m_pInstance)
        return;

    std::lock_guard lock(m_mtxCreate);
    delete m_pInstance;
    m_pInstance = nullptr;
}

void Logger::init(const std::string& loggerName, Logger::LogLevel level, bool enableConsole, bool isSync,
                  const std::string& filePath, size_t maxFileSize, size_t maxFiles)
{
    if (pimpl_->logger_)
        return;

    std::vector<spdlog::sink_ptr> sinks;
    if (enableConsole)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
    }
    if (!filePath.empty())
    {
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, maxFileSize, maxFiles));
    }

    if (isSync)
        initSync(sinks, loggerName, level);
    else
        initAsync(sinks, loggerName, level);
}

void Logger::initSync(std::vector<spdlog::sink_ptr> sinks, const std::string& loggerName, LogLevel level)
{
    if (sinks.empty())
    {
        pimpl_->logger_ = spdlog::stdout_color_mt(loggerName);
        return;
    }

    if (sinks.size() == 1)
    {
        pimpl_->logger_ = std::make_shared<spdlog::logger>(loggerName, sinks[0]);
    }
    else
    {
        pimpl_->logger_ = std::make_shared<spdlog::logger>(loggerName, begin(sinks), end(sinks));
    }
    pimpl_->logger_->set_level(LoggerImpl::mapLogLevel(level));
    pimpl_->logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%^%l%$][TID:%t][%s:%#:%!] %v");
    spdlog::set_default_logger(pimpl_->logger_);
    pimpl_->logger_->flush_on(LoggerImpl::mapLogLevel(level));
}

void Logger::initAsync(std::vector<spdlog::sink_ptr> sinks, const std::string& loggerName, LogLevel level)
{
    if (sinks.empty())
    {
        pimpl_->logger_ = spdlog::stdout_color_mt(loggerName);
        return;
    }

    spdlog::init_thread_pool(8192, 1);
    if (sinks.size() == 1)
    {
        pimpl_->logger_ = std::make_shared<spdlog::async_logger>(loggerName, sinks[0], spdlog::thread_pool(),
                                                                 spdlog::async_overflow_policy::block);
    }
    else
    {
        pimpl_->logger_ = std::make_shared<spdlog::async_logger>(
            loggerName, begin(sinks), end(sinks), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    }

    spdlog::flush_every(std::chrono::milliseconds(500));
    pimpl_->logger_->set_level(LoggerImpl::mapLogLevel(level));
    pimpl_->logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%^%l%$][TID:%t][%s:%#:%!] %v");
    spdlog::set_default_logger(pimpl_->logger_);
    pimpl_->logger_->flush_on(LoggerImpl::mapLogLevel(level));
}

struct LogStreamImpl
{
    Logger::LogLevel level_;
    std::ostringstream ss_;
    std::shared_ptr<spdlog::logger> logger_;
    const char* file_;
    int line_;
    const char* func_;

    LogStreamImpl(Logger::LogLevel level, std::shared_ptr<spdlog::logger> logger, const char* file, int line,
                  const char* func)
        : level_(level)
        , logger_(std::move(logger))
        , file_(file)
        , line_(line)
        , func_(func)
    {
    }
    ~LogStreamImpl() = default;
};

Logger::LogStream::LogStream(Logger::LogLevel level, LoggerImpl* loggerImpl, const char* file, int line,
                             const char* func)
    : pimpl_(std::make_unique<LogStreamImpl>(level, loggerImpl->logger_, file, line, func))
{
}

Logger::LogStream::~LogStream()
{
    if (pimpl_ && pimpl_->logger_)
    {
        pimpl_->logger_->log({pimpl_->file_, pimpl_->line_, pimpl_->func_}, LoggerImpl::mapLogLevel(pimpl_->level_),
                             pimpl_->ss_.str());
    }
}

template<typename T>
Logger::LogStream& Logger::LogStream::operator<<(const T& val)
{
    if (pimpl_ && pimpl_->ss_)
    {
        pimpl_->ss_ << val;
    }
    return *this;
}

Logger::LogStream& Logger::LogStream::operator<<(std::ostream& (*manip)(std::ostream&))
{
    manip(pimpl_->ss_);
    return *this;
}

Logger::LogStream& Logger::LogStream::operator<<(const char* str)
{
    if (pimpl_ && pimpl_->ss_)
    {
        pimpl_->ss_ << str;
    }
    return *this;
}

Logger::LogStream Logger::log(LogLevel eLevel, const char* file, int line, const char* func)
{
    return LogStream(eLevel, pimpl_.get(), file, line, func);
}

Logger::LogStream Logger::trace(const char* file, int line, const char* func)
{
    return LogStream(TRACE_L, pimpl_.get(), file, line, func);
}

Logger::LogStream Logger::debug(const char* file, int line, const char* func)
{
    return LogStream(DEBUG_L, pimpl_.get(), file, line, func);
}

Logger::LogStream Logger::info(const char* file, int line, const char* func)
{
    return LogStream(INFO_L, pimpl_.get(), file, line, func);
}

Logger::LogStream Logger::warn(const char* file, int line, const char* func)
{
    return LogStream(WARN_L, pimpl_.get(), file, line, func);
}

Logger::LogStream Logger::error(const char* file, int line, const char* func)
{
    return LogStream(ERROR_L, pimpl_.get(), file, line, func);
}

Logger::LogStream Logger::critical(const char* file, int line, const char* func)
{
    return LogStream(CRITICAL_L, pimpl_.get(), file, line, func);
}

// 显式模板实例化
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const char&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const short&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const int&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const long&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const long long&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const unsigned char&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const unsigned short&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const unsigned int&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const unsigned long&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const unsigned long long&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const float&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const double&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const long double&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const bool&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const void* const&);
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const std::string&);

// C++17 string_view 支持
#if __cplusplus >= 201703L
#include <string_view>
template LIB_API Logger::LogStream& Logger::LogStream::operator<<(const std::string_view&);
#endif