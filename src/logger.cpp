/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        logger.cpp
*
*****************************************************************************/

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

ve::Logger::LogLevel ve::Logger::getMaximumLevel() const
{
    return m_maximalLogLevel;
}

void ve::Logger::incrementFrameNumber()
{
    m_currentFrameID++;
}

void ve::Logger::printLog(const std::string& msg, LogLevel level)
{
    if(m_maximalLogLevel < level)
        return;
    printLogBanner(level);
    printf("%s", msg.c_str());
}

void ve::Logger::flush()
{
    fflush(stdout);
}

void ve::Logger::printLogBanner(LogLevel level)
{
    printf("[Repeater]");
    switch(level)
    {
        case DEBUG_PER_FRAME_LOG:
            printf("[Frm: %u]", m_currentFrameID);
            break;
        default:
            break;
    }
}
