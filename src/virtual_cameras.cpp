#include "virtual_cameras.hpp"
#include <glm/gtx/transform.hpp>
#include <cassert>

using namespace ve;
const glm::mat4& Camera::getViewMatrix() const
{
    return m_viewMatrix;
}

const ViewportArea& Camera::getViewport() const
{
    return m_viewport;
}

void VirtualCameras::setupWindows(size_t count, float windowsPerWidth)
{
    assert(windowsPerWidth > 0);
    m_cameras.clear();
    m_CamerasPerWidth = windowsPerWidth;

    const auto step = 1.0/count;
    const auto middle = count/2;
    for(size_t i = 0; i < count; i++)
    {
        int multiple = i-middle;
        const auto angle = step*multiple;
        m_cameras.push_back(Camera(angle));
    }
    recalculateViewports();
    recalculateTransformations();
}

/// Recalculate if parameters has changed
void VirtualCameras::updateParamaters(const CameraParameters& p)
{
    if(m_parameters == p)
        return;
    m_parameters = p;
    /// Transformations are depedent on parameters
    recalculateTransformations();
}

void VirtualCameras::updateViewports(const ViewportArea viewport)
{
    if(m_viewport == viewport)
        return;
    m_viewport = viewport;
    recalculateViewports();
}

/// Get reference to all cameras' container
const VirtualCameras::StorageContainer VirtualCameras::getCameras() const
{
    return m_cameras;
}

std::pair<size_t, size_t> VirtualCameras::getCameraGridSetup() const
{
    const size_t tilesPerY = m_cameras.size()/m_CamerasPerWidth+ ((m_cameras.size()%m_CamerasPerWidth) > 0);
    return std::make_pair(m_CamerasPerWidth,tilesPerY);
}

void VirtualCameras::recalculateTransformations()
{
    for(auto& camera: m_cameras)
    {
        // Calculate paramaters
        const auto finalCameraAngleRadians = camera.m_angleY*m_parameters.m_angleMultiplier;
        const auto rotationMatrix = glm::rotate(finalCameraAngleRadians, glm::vec3(0,1,0));
        const auto T = glm::translate(glm::vec3(0.0f,0.0f,m_parameters.m_distance));
        const auto invT = glm::translate(glm::vec3(0.0f,0.0f,-m_parameters.m_distance));
        camera.m_viewMatrix = invT*rotationMatrix*T;
    }
}

void VirtualCameras::recalculateViewports()
{
    const auto setup = getCameraGridSetup();
    const auto& tilesPerX = setup.first;
    const size_t tilesPerY = setup.second;

    const size_t width = m_viewport.getWidth()/tilesPerX;
    const size_t height= m_viewport.getHeight()/tilesPerY;

    const auto& startX = m_viewport.getX();
    const auto& startY = m_viewport.getY();


    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        auto& camera = m_cameras[i];
        // Calculate subviewport based on global m_viewport
        size_t posX = i % tilesPerX;
        size_t posY = i / tilesPerX;
        
        // Invert upside-down => OpenGL starts (0,0) at the bottom of screen
        posY = tilesPerY-posY-1;

        const size_t currentStartX = startX + posX*width;
        const size_t currentStartY = startY + posY*height;
        camera.m_viewport.set(currentStartX, currentStartY, width, height);
    }
}
