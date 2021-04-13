/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        diagnostics.cpp
*
*****************************************************************************/

#include "diagnostics.hpp"
#include "logger.hpp"
#include <chrono>
#include <sstream>

using namespace ve;
void Diagnostics::setTerminationAfterFrame(size_t lastFrame)
{
    m_ExitAfterFrames = lastFrame;
}

void Diagnostics::incrementFrameCount()
{
    m_ElapsedFrames++;

    // Skip initial frames which may be full of resource-allocations and other time-demanding ops
    if (m_ElapsedFrames < 5)
        return;

    if (!m_shouldMeasureFPS)
        return;
    /* Store new time, compare it to previous and print out difference in ms */
    using namespace std::chrono;
    static high_resolution_clock::time_point lastFrame = high_resolution_clock::now();
    high_resolution_clock::time_point now = high_resolution_clock::now();

    auto framePeriodMs = duration_cast<std::chrono::microseconds>(now - lastFrame);
    std::stringstream ss;
    ss << "Frame ID: " << m_ElapsedFrames << " - " << framePeriodMs.count() << " us" << std::endl;
    Logger::log(ss.str());

    lastFrame = now;
}

bool Diagnostics::hasReachedLastFrame() const
{
    return (m_ElapsedFrames >= m_ExitAfterFrames && m_ExitAfterFrames != 0);
}

void Diagnostics::setScreenshotFormat(const std::string& format)
{
    m_ScreenshotNameFormat = format;
}

const std::string Diagnostics::getScreenshotName() const
{
    if (m_ScreenshotNameFormat.find("{}") != std::string::npos)
    {
        const auto position = m_ScreenshotNameFormat.find("{}");
        std::string outputName = m_ScreenshotNameFormat;
        return outputName.replace(position, 2, std::to_string(m_ElapsedFrames));
    }
    return m_ScreenshotNameFormat;
}

bool Diagnostics::shouldShowOnlySpecificVirtualCamera() const
{
    return (m_ActivatedCameraID != -1);
}

void Diagnostics::setOnlyVirtualCamera(size_t cameraID)
{
    m_ActivatedCameraID = cameraID;
}

size_t Diagnostics::getOnlyCameraID() const
{
    return m_ActivatedCameraID;
}

bool Diagnostics::shouldNotBeIntrusive() const
{
    return m_shouldNotBeIntrusive;
}

void Diagnostics::setNonIntrusiveness(bool shouldBeNonIntrusive)
{
    m_shouldNotBeIntrusive = shouldBeNonIntrusive;
}

void Diagnostics::setFPSMeasuringState(bool isOn)
{
    m_shouldMeasureFPS = isOn;
}
