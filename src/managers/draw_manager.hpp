#ifndef VE_DRAW_MANAGER_HPP
#define VE_DRAW_MANAGER_HPP

#include <functional>

namespace ve
{
class Context;
class PerspectiveProjectionParameters;

class DrawManager
{
    public:
    void draw(Context& context, const std::function<void(void)>& code);
    void setEnhancerDecodedProjection(Context& context, GLuint program, const PerspectiveProjectionParameters& projection);
    private:
    // TODO: Replace setters with separate class, devoted for intershader communication
    void setEnhancerShift(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust = 0.0);
    void resetEnhancerShift(Context& context);
    void setEnhancerIdentity(Context& context);
};
} // namespace ve

#endif
