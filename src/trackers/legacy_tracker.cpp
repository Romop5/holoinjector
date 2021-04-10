/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        trackers/legacy_tracker.cpp
*
*****************************************************************************/

#include "trackers/legacy_tracker.hpp"
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

using namespace ve;
using namespace ve::trackers;

bool LegacyTracker::isLegacyNeeded() const
{
    return m_isLegacyOpenGLUsed;
}
bool LegacyTracker::isOrthogonalProjection()
{
    return m_isOrthogonalProjection;
}

void LegacyTracker::matrixMode(GLenum type)
{
    m_isLegacyOpenGLUsed = true;
    m_currentMode = type;
}

GLenum LegacyTracker::getMatrixMode() const
{
    return m_currentMode;
}

void LegacyTracker::loadMatrix(const glm::mat4& m)
{
    m_isLegacyOpenGLUsed = true;
    switch (m_currentMode)
    {
    case GL_PROJECTION:
    {
        m_currentProjection = m;
        const auto& lastVector = glm::row(m_currentProjection, 3);
        m_isOrthogonalProjection = !(lastVector == glm::vec4(0, 0, -1, 0));
    }
    break;
    default:
    {
    }
    }
}

void LegacyTracker::multMatrix(const glm::mat4& m)
{
    m_isLegacyOpenGLUsed = true;
    switch (m_currentMode)
    {
    case GL_PROJECTION:
    {
        m_currentProjection = m_currentProjection * m;
        const auto& lastVector = glm::row(m_currentProjection, 3);
        m_isOrthogonalProjection = !(lastVector == glm::vec4(0, 0, -1, 0));
    }
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
