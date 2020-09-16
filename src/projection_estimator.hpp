#ifndef VE_PROJECTION_ESTIMATOR_HPP
#define VE_PROJECTION_ESTIMATOR_HPP
/**
 * @file projection_estimator.hpp
 * @brief Helper routines for estimating projection from MVP matrix
 * @author Roman Dobias
 * @date 2020-09-16
 */
#include <glm/glm.hpp>
namespace ve
{
    struct PerspectiveProjectionParameters
    {
        float fx; // field of view x
        float fy; // field of view y
        float nearPlane; // distance of near plane in view-space
        float farPlane; // distance of far plane in view-space
        float A; //matrix[2,2] => -2/(f-n)
        float B; //matrix[2,3] => -(f+n)/(f-n)
    };

    /**
     * @brief Estimates perspective's projection parameters
     *
     *
     * @param transformationMatrix (could be any of MVP, VP, or P)
     * @return estimated parameters
     */
    PerspectiveProjectionParameters estimatePerspectiveProjection(glm::mat4 transformationMatrix);
} // namespace ve
#endif
