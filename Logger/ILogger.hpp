
#pragma once

#include <algorithm>
#include <iostream>
#include <optional>
#include <unordered_map>
#include "ExportDefine.h"
#include "GlobalDefine.h"

namespace spdlog
{
class logger;
};

namespace CHUtils
{
class CLogger;

class LOGGER_API ILogger
{
public:
    static inline ILogger& s_CreateInstance()
    {
        if (!s_pInstance)
        {
            s_pInstance = new ILogger();
        }
        return *s_pInstance;
    }

    static inline void s_DestroyInstance()
    {
        if (s_pInstance)
        {
            delete s_pInstance;
            s_pInstance = nullptr;
        }
    }

    bool addLogger(const std::string& loggerName, const std::string& logFilePath);
    bool setDefaultLogger(const std::string& loggerName);
    std::vector<std::string> getAllLoggersName();
    bool info(const ST_LOG_INFO& loc, const char* message);
    std::shared_ptr<spdlog::logger> getlogger();

private:
    ILogger() = default;
    ~ILogger() = default;
    ILogger(const ILogger&) = delete;
    ILogger& operator=(const ILogger&) = delete;
    ILogger(ILogger&&) = delete;
    ILogger& operator=(ILogger&&) = delete;

private:
    std::unordered_map<std::string, std::shared_ptr<CLogger>> m_mapLogger;
    std::shared_ptr<CLogger> m_defaultLogger;
    static inline ILogger* s_pInstance = nullptr;
};

};  // namespace CHUtils