/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        diagnostics.hpp
*
*****************************************************************************/

#ifndef HI_DIAGNOSTICS_HPP
#define HI_DIAGNOSTICS_HPP
#include <cstddef>
#include <string>
namespace hi
{
/**
     * @brief Stores options regarding to system testing/diagnosis
     */
class Diagnostics
{
public:
    /// Schedule termination after lastFrame
    void setTerminationAfterFrame(size_t lastFrame);
    /// Increment the count of elapsed frames
    void incrementFrameCount();

    /// Screnshot format
    void setScreenshotFormat(const std::string& format);

    /// Get screenshot name
    const std::string getScreenshotName() const;

    /// If ExitAfterFrames is set, determines if we reached the limit
    bool hasReachedLastFrame() const;

    /// If toggled, don't render whole grid, but only on of cameras
    bool shouldShowOnlySpecificVirtualCamera() const;

    /// Returns camera ID
    size_t getOnlyCameraID() const;

    /// Override default multiple camera grid with a single camera
    void setOnlyVirtualCamera(size_t cameraID);

    bool shouldNotBeIntrusive() const;
    void setNonIntrusiveness(bool shouldBeNonIntrusive);
    void setFPSMeasuringState(bool isOn);

private:
    /// Debug option: count rendered frames
    size_t m_ElapsedFrames = 0;

    /// Debug option: terminate process after N frames (or go for infty in case of 0)
    size_t m_ExitAfterFrames = 0;

    /// May contain {} (for frame ID)
    std::string m_ScreenshotNameFormat = "screenshot_{}.bmp";

    /// Is specific camera activated
    size_t m_ActivatedCameraID = -1;

    /// Don't affect shaders, push transforms, etc.
    bool m_shouldNotBeIntrusive = false;

    /// Should measure FPS's time in each frame
    bool m_shouldMeasureFPS = false;
};
} // namespace hi
#endif
