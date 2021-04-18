/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/camera_parameters.hpp
*
*****************************************************************************/

#ifndef HI_CAMERA_PARAMETERS_HPP
#define HI_CAMERA_PARAMETERS_HPP

namespace hi
{
namespace pipeline
{

    struct CameraParameters
    {
        /// Multiplies camera's shift(disparity) in X
        float m_XShiftMultiplier = 0.0;
        /// Distance in view-space of the centrum where all optical axis points at
        float m_frontOpticalAxisCentreDistance = 1.0;

        /// Default constructor
        CameraParameters() = default;
        /// Copy-constructor
        CameraParameters(CameraParameters&) = default;

        bool operator==(const CameraParameters& p) const;
    };

} // namespace pipeline
} // namespace hi

#endif
