/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        pipeline/projection_estimator.hpp
*
*****************************************************************************/

#ifndef HI_PROJECTION_ESTIMATOR_HPP
#define HI_PROJECTION_ESTIMATOR_HPP
/**
 * @file pipeline/projection_estimator.hpp
 * @brief Helper routines for estimating projection from MVP matrix
 * @author Roman Dobias
 * @date 2020-09-16
 */
#include <glm/glm.hpp>
namespace hi
{
namespace pipeline
{
    struct PerspectiveProjectionParameters
    {
        bool isPerspective = true;

        float fx; // field of view x
        float fy; // field of view y
        float nearPlane = 0.0f; // distance of near plane in view-space
        float farPlane = 0.0f; // distance of far plane in view-space
        float A = 0.0f; //matrix[2,2] => -2/(f-n)
        float B = 0.0f; //matrix[2,3] => -(f+n)/(f-n)

        glm::vec4 asVector() const
        {
            return glm::vec4(fx, fy, nearPlane, farPlane);
        }
    };

    /**
     * @brief Estimates perspective's projection parameters
     *
     *
     * @param transformationMatrix (could be any of MVP, VP, or P)
     * @return estimated parameters
     */
    PerspectiveProjectionParameters estimatePerspectiveProjection(glm::mat4 transformationMatrix);
} // namespace pipeline
} // namespace hi
#endif
