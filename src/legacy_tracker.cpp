#include "legacy_tracker.hpp"
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

using namespace ve;

bool LegacyTracker::isLegacyNeeded() const
{
    return m_isLegacyOpenGLUsed;
}
bool LegacyTracker::isOrthogonalProjection()
{
    const auto& lastVector = glm::row(m_currentProjection, 3);
    if(lastVector == glm::vec4(0,0,-1,0))
        return false;
    return true;
}

GLenum LegacyTracker::getCurrentMode() const
{
    return m_currentMode;
}

void LegacyTracker::matrixMode(GLenum type)
{
    m_isLegacyOpenGLUsed = true;
    m_currentMode = type;
}

void LegacyTracker::loadMatrix(const glm::mat4&& projection)
{
    m_isLegacyOpenGLUsed = true;
    switch(m_currentMode)
    {
        case GL_PROJECTION:
            m_currentProjection = projection;
        break;
        default:
        {
        }
    }
}

const glm::mat4& LegacyTracker::getProjection() const
{
    return m_currentProjection;
}
