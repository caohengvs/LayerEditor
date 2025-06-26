// MyLogger.cpp
#include "MyLogger.hpp"

#include <sstream>
#include <vector>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

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
                return spdlog::level::info;  // 默认值
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
    static MyLogger instance;
    return instance;
}

void MyLogger::init(const std::string& loggerName, MyLogger::LogLevel level, bool enableConsole,
                    const std::string& filePath, size_t maxFileSize, size_t maxFiles)
{
    if (!pimpl_->logger_)
    {
        std::vector<spdlog::sink_ptr> sinks;
        if (enableConsole)
        {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }
        if (!filePath.empty())
        {
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, maxFileSize, maxFiles));
        }

        if (sinks.empty())
        {
            pimpl_->logger_ = spdlog::stdout_color_mt(loggerName);
        }
        else if (sinks.size() == 1)
        {
            pimpl_->logger_ = std::make_shared<spdlog::logger>(loggerName, sinks[0]);
        }
        else
        {
            pimpl_->logger_ = std::make_shared<spdlog::logger>(loggerName, begin(sinks), end(sinks));
        }
        pimpl_->logger_->set_level(MyLoggerImpl::mapLogLevel(level));
        spdlog::set_default_logger(pimpl_->logger_);
        pimpl_->logger_->flush_on(MyLoggerImpl::mapLogLevel(level));
    }
}

struct MyLogger::LogStreamImpl
{
    MyLogger::LogLevel level_;
    std::ostringstream ss_;
    std::shared_ptr<spdlog::logger> logger_;
    const char* file_;  // 新增
    int line_;          // 新增
    const char* func_;  // 新增
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

MyLogger::LogStream::LogStream(LogLevel level, MyLoggerImpl* loggerImpl, const char* file, int line, const char* func)
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

template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const char&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const short&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const int&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const long&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const long long&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned char&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned short&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned int&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned long&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const unsigned long long&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const float&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const double&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const long double&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const bool&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const void* const&);
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const std::string&);

// C++17 string_view
#if __cplusplus >= 201703L
#include <string_view>  // 仅在此处包含
template MYLOGGER_API MyLogger::LogStream& MyLogger::LogStream::operator<<(const std::string_view&);
#endif