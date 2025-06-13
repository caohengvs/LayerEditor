#include "ILogger.hpp"
#include "CLogger.hpp"
#include <spdlog/logger.h>

namespace CHUtils
{
bool ILogger::addLogger(const std::string& loggerName, const std::string& logFilePath)
{
    try
    {
        auto logger = std::make_shared<CLogger>(loggerName, logFilePath);

        auto [iter, inserted] = m_mapLogger.try_emplace(loggerName, logger);

        return inserted;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ILogger::setDefaultLogger(const std::string& loggerName)
{
    auto it = m_mapLogger.find(loggerName);
    if (it == m_mapLogger.end())
    {
        return false;  // Logger not found
    }

    m_defaultLogger = it->second;
    return true;
}

std::vector<std::string> ILogger::getAllLoggersName()
{
    std::vector<std::string> vecloggerNames;
    vecloggerNames.reserve(m_mapLogger.size());

    std::transform(m_mapLogger.begin(), m_mapLogger.end(), std::back_inserter(vecloggerNames),
                   [](const auto& pair) { return pair.first; });

    return vecloggerNames;
}

bool ILogger::info(const ST_LOG_INFO& loc, const char* message)
{
    if (!m_defaultLogger)
    {
        std::cerr << "No default logger set. Cannot log message." << std::endl;
        return false;
    }

    m_defaultLogger->log({loc.file, loc.line, loc.func}, spdlog::level::debug, message);
    return true;
}

std::shared_ptr<spdlog::logger> ILogger::getlogger()
{
    if (!m_defaultLogger)
        return nullptr;  // No default logger set

    return m_defaultLogger->getLogger();
}
};  // namespace CHUtils