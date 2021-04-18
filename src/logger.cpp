/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        logger.cpp
*
*****************************************************************************/

#include "logger.hpp"
#include <memory>

hi::Logger& hi::Logger::getInstance()
{
    static std::unique_ptr<hi::Logger> m_singleton = nullptr;
    if (!m_singleton)
    {
        m_singleton.reset(new hi::Logger());
    }
    return *m_singleton;
}

void hi::Logger::setMaximumLevel(hi::Logger::LogLevel level)
{
    m_maximalLogLevel = level;
}

hi::Logger::LogLevel hi::Logger::getMaximumLevel() const
{
    return m_maximalLogLevel;
}

void hi::Logger::incrementFrameNumber()
{
    m_currentFrameID++;
}

void hi::Logger::printLog(const std::string& msg, LogLevel level)
{
    if (m_maximalLogLevel < level)
        return;
    printLogBanner(level);
    printf("%s", msg.c_str());
}

void hi::Logger::flush()
{
    fflush(stdout);
}

void hi::Logger::printLogBanner(LogLevel level)
{
    printf("[Repeater]");
    switch (level)
    {
    case DEBUG_PER_FRAME_LOG:
        printf("[Frm: %u]", m_currentFrameID);
        break;
    default:
        break;
    }
}
