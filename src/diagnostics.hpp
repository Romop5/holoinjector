#ifndef VE_DIAGNOSTICS_HPP
#define VE_DIAGNOSTICS_HPP
#include <cstddef>
#include <string>
namespace ve
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


        /// If ExitAfterFrames is set, determines if we reached the limit
        bool hasReachedLastFrame() const;

        /// If toggled, don't render whole grid, but only on of cameras
        bool shouldShowOnlySpecificVirtualCamera() const;

        /// Returns camera ID
        size_t getOnlyCameraID() const;

        /// Override default multiple camera grid with a single camera
        void setOnlyVirtualCamera(size_t cameraID);

        /// Get screenshot name
        const std::string getScreenshotName() const;
        private:
        /// Debug option: count rendered frames
        size_t m_ElapsedFrames = 0;

        /// Debug option: terminate process after N frames (or go for infty in case of 0)
        size_t m_ExitAfterFrames = 0;

        /// May contain %d (for frame ID)
        std::string m_ScreenshotNameFormat = "screenshot_%d.bmp";

        /// Is specific camera activated
        size_t m_ActivatedCameraID = -1;
    };
} // namespace ve
#endif
