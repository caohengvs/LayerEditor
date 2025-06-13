#pragma once
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace CHUtils
{
class CLogger
{
public:
    CLogger(const std::string& loggerName, const std::string& logFilePath);
    ~CLogger();


    template <typename... Args> 
    inline void log(const spdlog::source_loc& loc, spdlog::level::level_enum lvl, const char* fmt, Args&&... args)
    {
        if (m_logger)
        {
            m_logger->log(loc, lvl, fmt, std::forward<Args>(args)...);
        }
    }

    std::shared_ptr<spdlog::logger> getLogger() const
    {
        return m_logger;
    }

private:
    CLogger(const CLogger&) = delete;
    CLogger& operator=(const CLogger&) = delete;
    CLogger(CLogger&&) = delete;
    CLogger& operator=(CLogger&&) = delete;

private:
    std::shared_ptr<spdlog::logger> m_logger;
    std::vector<spdlog::sink_ptr> m_sinks;
    bool m_isAsync;
};
};  // namespace CHUtils