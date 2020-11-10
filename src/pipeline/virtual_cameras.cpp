#include "pipeline/virtual_cameras.hpp"
#include <glm/gtx/transform.hpp>
#include <cassert>

using namespace ve;
const glm::mat4& Camera::getViewMatrix() const
{
    return m_viewMatrix;
}

const glm::mat4& Camera::getViewMatrixRotational() const
{
    return m_viewMatrixRotational;
}


const float Camera::getAngle() const
{
    return m_angleY;
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
        const auto angle = -step*multiple;
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
const VirtualCameras::StorageContainer& VirtualCameras::getCameras() const
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
        /*const auto finalCameraAngleRadians = camera.m_angleY*m_parameters.m_XShiftMultiplier;
        const auto rotationMatrix = glm::rotate(finalCameraAngleRadians, glm::vec3(0,1,0));
        const auto T = glm::translate(glm::vec3(0.0f,0.0f,m_parameters.m_frontOpticalAxisCentreDistance));
        const auto invT = glm::translate(glm::vec3(0.0f,0.0f,-m_parameters.m_frontOpticalAxisCentreDistance));
        */

        /*
         * Holo display expects translated cameras along X axis
         * + changed optical axis towards a single point in front of the original camera
         */
        const auto horizontalShift = camera.m_angleY*m_parameters.m_XShiftMultiplier;
        const auto T = glm::translate(glm::vec3(horizontalShift, 0.0f,0.0f));

        // Shift camera along X
        camera.m_viewMatrix = T;
        // Set identity in case of skybox
        camera.m_viewMatrixRotational = glm::mat4(1.0);
    }
}

void VirtualCameras::recalculateViewports()
{
    if(m_cameras.size() == 0)
        return;
    const auto setup = getCameraGridSetup();
    const auto& tilesPerX = setup.first;
    const size_t tilesPerY = setup.second;
    // Grid must have logical sizes in order to be constructible
    assert(tilesPerX > 0 && tilesPerY > 0);

    const size_t width = m_viewport.getWidth()/tilesPerX;
    const size_t height= m_viewport.getHeight()/tilesPerY;

    const auto& startX = m_viewport.getX();
    const auto& startY = m_viewport.getY();


    auto countOfTiles = m_cameras.size();
    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        auto& camera = m_cameras[i];
        // Calculate subviewport based on global m_viewport
        size_t posX = i % tilesPerX;
        // Invert upside-down => OpenGL starts (0,0) at the bottom of screen
        size_t posY = (countOfTiles-i-1) / tilesPerX;

        const size_t currentStartX = startX + posX*width;
        const size_t currentStartY = startY + posY*height;
        camera.m_viewport.set(currentStartX, currentStartY, width, height);
    }
}
