#ifndef VE_DRAW_MANAGER_HPP
#define VE_DRAW_MANAGER_HPP

#include <functional>

namespace ve
{
class Context;
namespace pipeline {
    class PerspectiveProjectionParameters;
}
namespace managers
{

class DrawPipelineInterface
{
    public:
    template<typename T>
    void setParameter(const std::string& name, const T value);

    void setSingleViewMode(size_t cameraID);
    void setMultiViewMode(size_t cameraID);
    void push();
    void pop();
};
class DrawManager
{
    public:
    void draw(Context& context, const std::function<void(void)>& code);
    void setEnhancerDecodedProjection(Context& context, GLuint program, const ve::pipeline::PerspectiveProjectionParameters& projection);
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
} // namespace managers
} // namespace ve

#endif
