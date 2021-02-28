#include "logger.hpp"
#include <memory>

ve::Logger& ve::Logger::getInstance()
{
    static std::unique_ptr<ve::Logger> m_singleton = nullptr;
    if(!m_singleton)
    {
        m_singleton.reset(new ve::Logger());
    }
    return *m_singleton;
}

void ve::Logger::setMaximumLevel(ve::Logger::LogLevel level)
{
    m_maximalLogLevel = level;
}

void ve::Logger::printLog(const std::string& msg, LogLevel level)
{
    if(m_maximalLogLevel < level)
        return;
    printf("%s", msg.c_str());
}

void ve::Logger::flush()
{
    fflush(stdout);
}
