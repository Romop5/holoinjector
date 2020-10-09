#include "diagnostics.hpp"

using namespace ve;
void Diagnostics::setTerminationAfterFrame(size_t lastFrame)
{
    m_ExitAfterFrames = lastFrame;
}

void Diagnostics::incrementFrameCount()
{
    m_ElapsedFrames++;
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
    if(m_ScreenshotNameFormat.find("%d") != std::string::npos)
    {
        const auto position = m_ScreenshotNameFormat.find("%d");
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

