#include "CLogger.hpp"
#include <iostream>
namespace CHUtils
{
CLogger::CLogger(const std::string& loggerName, const std::string& logFilePath)
    : m_isAsync(true)
{
    auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);

    m_sinks = {fileSink, stdoutSink};

    try
    {
        m_logger = spdlog::get(loggerName);
        if (m_logger)
        {
            return;
        }

        if (m_isAsync)
        {
            spdlog::init_thread_pool(8192, 1);
            m_logger =
                std::make_shared<spdlog::async_logger>(loggerName, m_sinks.begin(), m_sinks.end(),
                                                       spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        }
        else
        {
            m_logger = std::make_shared<spdlog::logger>(loggerName, m_sinks.begin(), m_sinks.end());
        }

        spdlog::flush_every(std::chrono::milliseconds(500));
        spdlog::register_logger(m_logger);

        m_logger->set_level(spdlog::level::debug);
        m_logger->flush_on(spdlog::level::debug);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
    }
}

CLogger::~CLogger()
{
    spdlog::shutdown();
}
};  // namespace CHUtils