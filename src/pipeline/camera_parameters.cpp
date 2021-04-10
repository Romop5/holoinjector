/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/camera_parameters.cpp
*
*****************************************************************************/

#include "pipeline/camera_parameters.hpp"

using namespace ve;
using namespace ve::pipeline;

bool CameraParameters::operator==(const CameraParameters& p) const
{
    return (m_XShiftMultiplier == p.m_XShiftMultiplier
        && m_frontOpticalAxisCentreDistance == p.m_frontOpticalAxisCentreDistance);
}
