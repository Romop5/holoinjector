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
    /// Decide which draw methods should be used
    void drawGeneric(Context& context, const std::function<void(void)>& code);
    /// Draw without support of GS, or when shaderless fixed-pipeline is used
    void drawLegacy(Context& context, const std::function<void(void)>& code);
    /// Draw when Geometry Shader has been injected into program
    void drawWithGeometryShader(Context& context, const std::function<void(void)>& code);

    // TODO: Replace setters with separate class, devoted for intershader communication
    void setEnhancerShift(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust = 0.0);
    void resetEnhancerShift(Context& context);
    void setEnhancerIdentity(Context& context);

    void pushFixedPipelineProjection(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust = 0.0);
    void popFixedPipelineProjection(Context& context);


    // Legacy OpenGL - Create single-view FBO from current FBO
    GLuint createSingleViewFBO(Context& contex, size_t layer);
};
} // namespace ve

#endif
