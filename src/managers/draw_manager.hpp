/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        managers/draw_manager.hpp
*
*****************************************************************************/

#ifndef HI_DRAW_MANAGER_HPP
#define HI_DRAW_MANAGER_HPP

#include <functional>

namespace hi
{
class Context;

namespace pipeline
{
    class PerspectiveProjectionParameters;
}
namespace managers
{

    class DrawManager
    {
    public:
        void draw(Context& context, const std::function<void(void)>& code);
        void setInjectorDecodedProjection(Context& context, GLuint program, const hi::pipeline::PerspectiveProjectionParameters& projection);

    private:
        /// Decide if current draw call is dispached in suitable settings
        bool shouldSkipDrawCall(Context& context);
        /// Decide which draw methods should be used
        void drawGeneric(Context& context, const std::function<void(void)>& code);
        /// Draw without support of GS, or when shaderless fixed-pipeline is used
        void drawLegacy(Context& context, const std::function<void(void)>& code);
        /// Draw when Geometry Shader has been injected into program
        void drawWithGeometryShader(Context& context, const std::function<void(void)>& code);
        /// Draw without, just using Vertex Shader + repeating
        void drawWithVertexShader(Context& context, const std::function<void(void)>& code);

        std::string dumpDrawContext(Context& context) const;

        // TODO: Replace setters with separate class, devoted for intershader communication
        void setInjectorShift(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust = 0.0);
        void resetInjectorShift(Context& context);
        void setInjectorIdentity(Context& context);

        void pushFixedPipelineProjection(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust = 0.0);
        void popFixedPipelineProjection(Context& context);

        // Legacy OpenGL - Create single-view FBO from current FBO
        GLuint createSingleViewFBO(Context& contex, size_t layer);

        /* Context queries */
        bool isSingleViewPossible(Context& context);
        bool isRepeatingSuitable(Context& context);

        void setInjectorUniforms(size_t shaderID, Context& context);
    };
} // namespace managers
} // namespace hi

#endif
