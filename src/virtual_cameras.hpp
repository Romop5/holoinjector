#ifndef ENHANCER_VIRTUAL_CAMERAS_HPP
#define ENHANCER_VIRTUAL_CAMERAS_HPP
#include <vector>
#include <glm/glm.hpp>
#include "viewport_area.hpp"

namespace ve
{
    class Camera
    {
        public:
        Camera(float initialAngle): m_angleY(initialAngle) {}
        const glm::mat4& getViewMatrix() const;
        const ViewportArea& getViewport() const;
        friend class VirtualCameras;
        protected:
        /// Constant, defining initial angle around axis Y
        const float m_angleY;
        /// Cache: world-to-viewspace
        glm::mat4 m_viewMatrix;
        /// Cache: per-camera viewport
        ViewportArea m_viewport;
    };

    /// Groups parameters
    struct CameraParameters
    {
        float m_angleMultiplier = 0.0;
        float m_distance = 1.0;
        /// Default constructor
        CameraParameters() = default;
        /// Copy-constructor
        CameraParameters(CameraParameters&) = default;
        bool operator==(const CameraParameters& p)
        {
            return (m_angleMultiplier == p.m_angleMultiplier
                    && m_distance == p.m_distance);
        }
    };

    /*
     * \brief Stores cached parameters for all virtual cameras
     *
     * When rendering into multiple views, matrix transformations are cached
     * to save CPU time. These are recalculated when stored CameraParameters
     * differs from the one, passed to updateWindows() method.
     */
    class VirtualCameras
    {
        public:
        using StorageContainer = std::vector<Camera>;
        
        /// Create 'count' windows with different angle, arranged into grid according to 'windowsPerWidth'
        void setupWindows(size_t count, float windowsPerWidth);
        /// Recalculate if parameters has changed
        void updateParamaters(const CameraParameters& newParameters);
        void updateViewports(const ViewportArea viewport);

        /// Get reference to all cameras' container
        const StorageContainer getCameras() const;
        
        /// Returns (per-width,per-height) camera counts
        std::pair<size_t, size_t> getCameraGridSetup() const;
        protected:
        void recalculateTransformations();
        void recalculateViewports();
        private:
        ViewportArea m_viewport;
        CameraParameters m_parameters;
        /// Stores cameras in STL container
        StorageContainer m_cameras;
        size_t m_CamerasPerWidth = 1;
    };
} //namespace ve
#endif
