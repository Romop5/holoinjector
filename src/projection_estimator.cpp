#include "projection_estimator.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>  // glm::translate etc.

#include <glm/gtc/matrix_access.hpp> // glm::row
#include <glm/gtx/norm.hpp>      // norms
#include <glm/gtc/constants.hpp> // epsilon



using namespace ve;

namespace helper
{
    bool isMVPwithoutScale(const glm::mat4& mvp)
    {
        // a scale-less MVP has norm(a[3][0:3]) equal 1
        return (std::abs(glm::length(glm::vec3(glm::row(mvp,3))))-1.0) < glm::epsilon<float>();
    }

    /// Given A and B of perspective projection, estimate far/near plane distances
    glm::vec2 solveFarNearFromAB(double A, double B)
    {
        // See repoRoot/derivation/projection.tex for explanation of this derivation
        // near plane distance
        double near = (-B)/(-A+1.0);
        // range = (far-near)
        double range =  -(near*near)/(near+B*0.5);
        return glm::vec2(static_cast<float>(near), static_cast<float>(near+range));
    }

    /**
     * @brief Only rescale when really needed, keaping floating-point precision
     */
    template<typename T>
    inline void rescaleIfNecessary(T& value, T scale, bool shouldRescale)
    {
        if(shouldRescale)
        {
            value /=scale;
        }
    }

    PerspectiveProjectionParameters extractPerspective(const glm::mat4& mvp)
    {
        PerspectiveProjectionParameters result;
        // Obtain fx as length of 0th row of 3x3 matrix
        result.fx = (glm::length(glm::vec3(glm::row(mvp,0))));
        // Obtain fy as length of 0th row of 3x3 matrix
        result.fy = (glm::length(glm::vec3(glm::row(mvp,1))));

        // If scale transformation was used, 3x3 matrix is scaled. 
        // This can be corrected perfectly provided a single scalar was used for scaling
        auto scalingCorrection = (glm::length2(glm::vec3(glm::row(mvp,3))));
        if(scalingCorrection < 1e-6)
        {
            // for orthogonal projection, we can't estimate depth => use fixed depth
            result.isPerspective = false;
            return result;
        }

        // Only rescale if scale is significantly different from 1.0 to preserve precision
        bool needsScalingCorrection = ((scalingCorrection-1.0) > 1e-6);
        if(needsScalingCorrection && result.isPerspective)
        {
            auto realScale = (glm::length(glm::vec3(glm::row(mvp,3))));
            scalingCorrection = realScale;
        }
        // Correct fx/fy 
        helper::rescaleIfNecessary(result.fx, scalingCorrection, needsScalingCorrection);
        helper::rescaleIfNecessary(result.fy, scalingCorrection, needsScalingCorrection);

        /*
         * Obtaining A and B
         * where A = -2/(f-n)
         *       B = -(f+n)/(f-n)
         * is done as reverting multiplication of projection and transformation
         */
        // Note that A is always minus because range is always positive, because by 
        // definition, far plane is further than near plane => range must be positive
        result.A = -(glm::length(glm::vec3(glm::row(mvp,2))));
        helper::rescaleIfNecessary(result.A, scalingCorrection, needsScalingCorrection);
        auto l = -mvp[3][3];
        helper::rescaleIfNecessary(l, scalingCorrection, needsScalingCorrection);
        auto tmp = mvp[3][2];
        result.B = tmp-l*result.A;

        // Derive near/far from A and B, defined above, by plugging into equation
        auto farNearVector = solveFarNearFromAB(result.A, result.B);
        result.nearPlane = farNearVector[0];
        result.farPlane = farNearVector[1];
        return result;
    }
} //namespace helper


PerspectiveProjectionParameters ve::estimatePerspectiveProjection(glm::mat4 transformationMatrix)
{
    return helper::extractPerspective(transformationMatrix);
}

