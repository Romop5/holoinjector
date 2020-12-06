#include "pipeline/camera_parameters.hpp"

using namespace ve;
using namespace ve::pipeline;

bool CameraParameters::operator==(const CameraParameters& p) const
{
    return (m_XShiftMultiplier == p.m_XShiftMultiplier
            && m_frontOpticalAxisCentreDistance == p.m_frontOpticalAxisCentreDistance);
}
