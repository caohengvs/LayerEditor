#include "MyLogger.hpp"
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <vector>

struct MyLogger::MyLoggerImpl
{
    std::shared_ptr<spdlog::logger> logger_;

    static spdlog::level::level_enum mapLogLevel(MyLogger::LogLevel level)
    {
        switch (level)
        {
            case MyLogger::TRACE_L:
                return spdlog::level::trace;
            case MyLogger::DEBUG_L:
                return spdlog::level::debug;
            case MyLogger::INFO_L:
                return spdlog::level::info;
            case MyLogger::WARN_L:
                return spdlog::level::warn;
            case MyLogger::ERROR_L:
                return spdlog::level::err;
            case MyLogger::CRITICAL_L:
                return spdlog::level::critical;
            case MyLogger::OFF_L:
                return spdlog::level::off;
            default:
                return spdlog::level::info;
        }
    }
};

MyLogger::MyLogger()
    : pimpl_(std::make_unique<MyLoggerImpl>())
{
}

MyLogger::~MyLogger() = default;

MyLogger& MyLogger::getInstance()
{
    if (m_pInstance)
        return *m_pInstance;

    std::lock_guard lock(m_mtxCreate);
    m_pInstance = new MyLogger;
    return *m_pInstance;
}

void MyLogger::deleteInstance()
{
    if (!m_pInstance)
        return;

    std::lock_guard lock(m_mtxCreate);
    delete m_pInstance;
    m_pInstance = nullptr;
}

void MyLogger::init(const std::string& loggerName, MyLogger::LogLevel level, bool enableConsole,
                    const std::string& filePath, size_t maxFileSize, size_t maxFiles, bool isSync)
{
    if (pimpl_->logger_)
        return;

    std::vector<spdlog::sink_ptr> sinks;
    if (enableConsole)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
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

void MyLogger::initSync(std::vector<spdlog::sink_ptr> sinks, const std::string& loggerName, LogLevel level)
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
    pimpl_->logger_->set_level(MyLoggerImpl::mapLogLevel(level));
    pimpl_->logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%^%l%$][TID:%t][%s:%#:%!] %v");
    spdlog::set_default_logger(pimpl_->logger_);
    pimpl_->logger_->flush_on(MyLoggerImpl::mapLogLevel(level));
}

void MyLogger::initAsync(std::vector<spdlog::sink_ptr> sinks, const std::string& loggerName, LogLevel level)
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
    pimpl_->logger_->set_level(MyLoggerImpl::mapLogLevel(level));
    pimpl_->logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%^%l%$][TID:%t][%s:%#:%!] %v");
    spdlog::set_default_logger(pimpl_->logger_);
    pimpl_->logger_->flush_on(MyLoggerImpl::mapLogLevel(level));
}

struct MyLogger::LogStreamImpl
{
    MyLogger::LogLevel level_;
    std::ostringstream ss_;
    std::shared_ptr<spdlog::logger> logger_;
    const char* file_;
    int line_;
    const char* func_;

    LogStreamImpl(MyLogger::LogLevel level, std::shared_ptr<spdlog::logger> logger, const char* file, int line,
                  const char* func)
        : level_(level)
        , logger_(std::move(logger))
        , file_(file)
        , line_(line)
        , func_(func)
    {
    }
};

MyLogger::LogStream::LogStream(MyLogger::LogLevel level, MyLoggerImpl* loggerImpl, const char* file, int line,
                               const char* func)
    : pimpl_(std::make_unique<LogStreamImpl>(level, loggerImpl->logger_, file, line, func))
{
}

MyLogger::LogStream::~LogStream()
{
    if (pimpl_ && pimpl_->logger_)
    {
        pimpl_->logger_->log({pimpl_->file_, pimpl_->line_, pimpl_->func_},
                             MyLogger::MyLoggerImpl::mapLogLevel(pimpl_->level_), pimpl_->ss_.str());
    }
}

template<typename T>
MyLogger::LogStream& MyLogger::LogStream::operator<<(const T& val)
{
    if (pimpl_ && pimpl_->ss_)
    {
        pimpl_->ss_ << val;
    }
    return *this;
}

MyLogger::LogStream& MyLogger::LogStream::operator<<(std::ostream& (*manip)(std::ostream&))
{
    manip(pimpl_->ss_);
    return *this;
}

MyLogger::LogStream& MyLogger::LogStream::operator<<(const char* str)
{
    if (pimpl_ && pimpl_->ss_)
    {
        pimpl_->ss_ << str;
    }
    return *this;
}

MyLogger::LogStream MyLogger::log(LogLevel eLevel, const char* file, int line, const char* func)
{
    return LogStream(eLevel, pimpl_.get(), file, line, func);
}

MyLogger::LogStream MyLogger::trace(const char* file, int line, const char* func)
{
    return LogStream(TRACE_L, pimpl_.get(), file, line, func);
}

MyLogger::LogStream MyLogger::debug(const char* file, int line, const char* func)
{
    return LogStream(DEBUG_L, pimpl_.get(), file, line, func);
}

MyLogger::LogStream MyLogger::info(const char* file, int line, const char* func)
{
    return LogStream(INFO_L, pimpl_.get(), file, line, func);
}

MyLogger::LogStream MyLogger::warn(const char* file, int line, const char* func)
{
    return LogStream(WARN_L, pimpl_.get(), file, line, func);
}

MyLogger::LogStream MyLogger::error(const char* file, int line, const char* func)
{
    return LogStream(ERROR_L, pimpl_.get(), file, line, func);
}

MyLogger::LogStream MyLogger::critical(const char* file, int line, const char* func)
{
    return LogStream(CRITICAL_L, pimpl_.get(), file, line, func);
}

// 显式模板实例化
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const char&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const short&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const int&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const long&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const long long&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned char&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned short&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned int&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned long&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned long long&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const float&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const double&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const long double&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const bool&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const void* const&);
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const std::string&);

// C++17 string_view 支持
#if __cplusplus >= 201703L
#include <string_view>
template LIB_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const std::string_view&);
#endif