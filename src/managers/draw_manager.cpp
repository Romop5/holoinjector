/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        managers/draw_manager.cpp
*
*****************************************************************************/

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include <glm/gtc/type_ptr.hpp>

#include "context.hpp"
#include "draw_manager.hpp"
#include "logger.hpp"
#include "pipeline/camera_parameters.hpp"
#include "pipeline/output_fbo.hpp"
#include "pipeline/projection_estimator.hpp"
#include "pipeline/virtual_cameras.hpp"
#include "trackers/framebuffer_tracker.hpp"
#include "trackers/legacy_tracker.hpp"
#include "trackers/shader_tracker.hpp"
#include "trackers/texture_tracker.hpp"
#include "trackers/uniform_block_tracing.hpp"

#include "utils/opengl_debug.hpp"

using namespace hi;
using namespace hi::managers;

namespace helpers
{
namespace uniforms
{
    /*
         * \brief Injector internal: don't repeat draw calls and render to a single output layer of
         * bounded FBO
         */
    void renderToSingleLayer(size_t shaderId, size_t layerID)
    {
        CLEAR_GL_ERROR();
        auto loc = glGetUniformLocation(shaderId, "injector_isSingleViewActivated");
        glUniform1i(loc, true);
        ASSERT_GL_ERROR();

        loc = glGetUniformLocation(shaderId, "injector_singleViewID");
        glUniform1i(loc, layerID);
        ASSERT_GL_ERROR();
    }
    /*
         * \brief Injector internal: replicate transformed geometry to all output layers of bound FBO
         */
    void renderToAllLayers(size_t shaderId)
    {
        auto loc = glGetUniformLocation(shaderId, "injector_isSingleViewActivated");
        glUniform1i(loc, false);
        ASSERT_GL_ERROR();
    }
}
}

void DrawManager::draw(Context& context, const std::function<void(void)>& drawCallLambda)
{
    // Determine if shader is bound, if FBO is correctly bound, etc
    if (shouldSkipDrawCall(context))
        return;

    if (!context.m_IsMultiviewActivated || (context.getFBOTracker().hasBounded() && !context.getFBOTracker().isSuitableForRepeating()))
    {
        setInjectorIdentity(context);
        drawGeneric(context, drawCallLambda);
        return;
    }

    /*
     * If applications is drawing into offscreen FBO, then bind our shadowed FBO
     */
    if (!context.getFBOTracker().hasBounded())
    {
        assert(context.getOutputFBO().getFBOId() != 0 && "OutputFBO must be initialized");
        context.getOutputFBO().setContainsImageFlag();
        glBindFramebuffer(GL_FRAMEBUFFER, context.getOutputFBO().getFBOId());
        glViewport(0, 0, context.getOutputFBO().getParams().getTextureWidth(), context.getOutputFBO().getParams().getTextureHeight());
    }
    /*else {
        auto fbo = context.getFBOTracker().getBound();
        auto fboId = context.getFBOTracker().getBoundId();
        if(context.getFBOTracker().isSuitableForRepeating())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, (fbo->hasShadowFBO()? fbo->getShadowFBO() : fboId));
            if(!fbo->hasShadowFBO())
            {
                Logger::logError("Shadow FBO not found for given FBO -> draw call may not effect output");
            }
        }
    }*/

    /// If Uniform Buffer Object (UBO) is used, then load values to uniforms
    if (context.getManager().hasBounded() && context.getManager().getBound()->m_Metadata && context.getManager().getBound()->m_Metadata->isUBOused())
    {
        const auto& blockName = context.getManager().getBound()->m_Metadata->m_InterfaceBlockName;
        auto index = context.getManager().getBound()->m_UniformBlocks[blockName].bindingIndex;
        const auto& indexStructure = context.getUniformBlocksTracker().getBindingIndex(index);
        if (indexStructure.hasTransformation)
        {
            setInjectorDecodedProjection(context, context.getManager().getBoundId(), indexStructure.projection);
        }
        else
        {
            Logger::log("Unexpected state. Expected UBO, but not found. Falling back to identity\n");
            setInjectorIdentity(context);
        }
    }
    /*
     *  Load enhancer's values into uniforms
     */
    if (context.getManager().hasBounded())
    {
        const auto shaderID = context.getManager().getBoundId();
        setInjectorUniforms(shaderID, context);
    }
    drawGeneric(context, drawCallLambda);
    return;
}

bool DrawManager::shouldSkipDrawCall(Context& context)
{
    if (context.getManager().hasBounded())
    {
        const auto& program = context.getManager().getBound();
        if (program->hasMetadata() && program->m_Metadata->m_IsInvisible)
            return true;
    }
    if (context.m_IsMultiviewActivated && isRepeatingSuitable(context) && (!isSingleViewPossible(context)))
    {
        Logger::logDebug("Shadowing not possible -> terminating draw call");
        //return true;
    }
    return false;
}

void DrawManager::drawGeneric(Context& context, const std::function<void(void)>& drawCallLambda)
{
    const auto& programs = context.getManager();
    /// If application is using shaders and shader program has enhancer's GS capabilities
    if (programs.hasBounded() && programs.getBoundConst()->isInjected())
    {
        const auto& metadata = *programs.getBoundConst()->m_Metadata;
        if (metadata.usesGeometryShader())
        {
            drawWithGeometryShader(context, drawCallLambda);
        }
        else
        {
            drawWithVertexShader(context, drawCallLambda);
        }
    }
    else
    {
        drawLegacy(context, drawCallLambda);
    }
}

void DrawManager::drawWithGeometryShader(Context& context, const std::function<void(void)>& drawCallLambda)
{
    debug::logTrace("drawWithGeometryShader");
    if (!context.m_IsMultiviewActivated || !isSingleViewPossible(context))
    {
        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawGS: multiview off", HI_POS);
        return;
    }

    if (!context.getTextureTracker().getTextureUnits().hasShadowedTextureBinded())
    {
        helpers::uniforms::renderToAllLayers(context.getManager().getBoundId());
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawGS: single view", HI_POS);
        return;
    }

    const auto numOfLayers = context.getOutputFBO().getParams().getLayers();
    for (size_t l = 0; l < numOfLayers; l++)
    {
        context.getTextureTracker().getTextureUnits().bindShadowedTexturesToLayer(l);

        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), l);
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawGS: layer ", l, HI_POS);
    }
    context.getTextureTracker().getTextureUnits().unbindShadowedTextures();
}

void DrawManager::drawWithVertexShader(Context& context, const std::function<void(void)>& drawCallLambda)
{
    debug::logTrace("drawWithVertexShader");
    const auto middleCamera = (context.getCameras().getCameras().size() / 2);
    if (!context.m_IsMultiviewActivated || !isSingleViewPossible(context))
    {
        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawVS: non-multiview", HI_POS);
        return;
    }

    if (context.getFBOTracker().hasBounded() && !context.getFBOTracker().isSuitableForRepeating())
    {
        context.getTextureTracker().getTextureUnits().bindShadowedTexturesToLayer(middleCamera);

        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawVS: single", HI_POS);
        return;
    }
    for (size_t cameraID = 0; cameraID < context.getCameras().getCameras().size(); cameraID++)
    {
        // Bind correct layered texture
        context.getTextureTracker().getTextureUnits().bindShadowedTexturesToLayer(cameraID);

        // Create & set single view for current render
        auto shadowFBO = createSingleViewFBO(context, cameraID);
        assert(shadowFBO != 0);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

        // Single-view FBO only contains a single layer => 0
        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);

        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawVS: layer: ", cameraID,
            "shadowFBO: ", shadowFBO, HI_POS);
    }
    context.getTextureTracker().getTextureUnits().unbindShadowedTextures();
}

void DrawManager::drawLegacy(Context& context, const std::function<void(void)>& drawCallLambda)
{
    const auto middleCamera = (context.getCameras().getCameras().size() / 2);
    if (!context.m_IsMultiviewActivated || !isSingleViewPossible(context))
    {
        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawLegacy: non-multiview", HI_POS);
        return;
    }
    if ((context.getFBOTracker().hasBounded() && !context.getFBOTracker().isSuitableForRepeating()))
    {
        context.getTextureTracker().getTextureUnits().bindShadowedTexturesToLayer(0);
        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawLegacy: single", HI_POS);
        return;
    }

    for (size_t cameraID = 0; cameraID < context.getCameras().getCameras().size(); cameraID++)
    {
        context.getTextureTracker().getTextureUnits().bindShadowedTexturesToLayer(cameraID);
        const auto& camera = context.getCameras().getCameras()[cameraID];

        auto shadowFBO = createSingleViewFBO(context, cameraID);
        assert(shadowFBO != 0);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

        helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);

        const auto& t = camera.getViewMatrix();
        setInjectorShift(context, t, camera.getAngle() * context.getCameraParameters().m_XShiftMultiplier / context.getCameraParameters().m_frontOpticalAxisCentreDistance);
        drawCallLambda();
        Logger::logDebugPerFrame(dumpDrawContext(context), "drawLegacy: layer", cameraID,
            "shadowFBO: ", shadowFBO, HI_POS);
        resetInjectorShift(context);
    }
    context.getTextureTracker().getTextureUnits().unbindShadowedTextures();
}

std::string DrawManager::dumpDrawContext(Context& context) const
{
    std::stringstream ss;
    ss << "[FBO: " << context.getFBOTracker().getBoundId() << "] ";
    ss << "[Program: " << context.getManager().getBoundId() << "] ";
    return ss.str();
}
/*
 * ----------------------------------------------------------------------------
 */

void DrawManager::setInjectorShift(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust)
{
    const auto& resultMat = viewSpaceTransform;
    auto program = context.getManager().getBoundId();
    if (program)
    {
        //helpers::uniforms::renderToSingleLayer(context.getManager().getBoundId(), 0);

        auto location = glGetUniformLocation(program, "injector_identity");
        glUniform1i(location, GL_TRUE);
    }

    // Legacy support
    /*
     * multiply GL_PROJECTION from right
     * Note: GL_PROJECTION_STACK_DEPTH must be at least 2
     *
     * Note: sometimes GL_PROJECTION may not be well-shaped
     * e.g. when porting DirectX game to OpenGL
     */

    if (context.getLegacyTracker().isLegacyNeeded())
    {
        pushFixedPipelineProjection(context, resultMat, projectionAdjust);
    }
}

void DrawManager::resetInjectorShift(Context& context)
{
    if (context.getLegacyTracker().isLegacyNeeded())
    {
        popFixedPipelineProjection(context);
    }
}

void DrawManager::setInjectorIdentity(Context& context)
{
    const auto identity = glm::mat4(1.0);
    auto program = context.getManager().getBoundId();
    if (program == 0)
        return;
    auto location = glGetUniformLocation(program, "injector_identity");
    glUniform1i(location, GL_TRUE);

    location = glGetUniformLocation(program, "injector_max_views");
    glUniform1i(location, 1);
    location = glGetUniformLocation(program, "injector_max_invocations");
    glUniform1i(location, 1);
}

void DrawManager::setInjectorDecodedProjection(Context& context, GLuint program, const hi::pipeline::PerspectiveProjectionParameters& projection)
{
    // upload parameters to GPU's program
    auto parametersLocation = glGetUniformLocation(program, "injector_deprojection");
    glUniform4fv(parametersLocation, 1, glm::value_ptr(projection.asVector()));

    parametersLocation = glGetUniformLocation(program, "injector_deprojection_inv");
    glm::vec4 inverted = glm::vec4(1.0) / projection.asVector();
    glUniform4fv(parametersLocation, 1, glm::value_ptr(inverted));

    auto typeLocation = glGetUniformLocation(program, "injector_isOrthogonal");
    glUniform1i(typeLocation, !projection.isPerspective);
}

void DrawManager::pushFixedPipelineProjection(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust)
{
    glMatrixMode(GL_PROJECTION);
    auto oldProjection = context.getLegacyTracker().getProjection();
    oldProjection[2][0] = projectionAdjust;
    const auto newProjection = oldProjection * viewSpaceTransform;
    if (!context.getLegacyTracker().isOrthogonalProjection())
        glLoadMatrixf(glm::value_ptr(newProjection));
}
void DrawManager::popFixedPipelineProjection(Context& context)
{
    glLoadMatrixf(glm::value_ptr(context.getLegacyTracker().getProjection()));
    glMatrixMode(context.getLegacyTracker().getMatrixMode());
}

GLuint DrawManager::createSingleViewFBO(Context& context, size_t layer)
{
    if (context.getFBOTracker().hasBounded())
    {
        auto& fbo = context.getFBOTracker().getBound();
        if (context.m_IsMultiviewActivated)
        {
            if (fbo->hasShadowFBO())
            {
                return fbo->createProxyFBO(layer);
            }
            else
            {
                if (fbo->hasFailedToCreateShadowFBO())
                {
                    Logger::logDebug("Drawing to FBO without shadow FBO due to failed init", HI_POS);
                }
                else
                {
                    Logger::logError("Single-layer proxy FBO failed due to missing Shadow FBO");
                }
            }
        }
        return context.getFBOTracker().getBoundId();
    }
    else
    {
        return context.getOutputFBO().createProxyFBO(layer);
    }
}

bool DrawManager::isSingleViewPossible(Context& context)
{
    if (context.getFBOTracker().hasBounded())
    {
        auto& fbo = context.getFBOTracker().getBound();
        if (fbo->hasShadowFBO())
        {
            return true;
        }
        return false;
    }
    // For OutputFBO, it's always true
    return true;
}

bool DrawManager::isRepeatingSuitable(Context& context)
{
    if (context.getFBOTracker().hasBounded())
    {
        return context.getFBOTracker().isSuitableForRepeating();
    }
    return true;
}

void DrawManager::setInjectorUniforms(size_t shaderID, Context& context)
{
    assert(context.getManager().hasBounded());

    auto loc = glGetUniformLocation(context.getManager().getBoundId(), "injector_XShiftMultiplier");
    glUniform1f(loc, context.getCameraParameters().m_XShiftMultiplier);

    loc = glGetUniformLocation(context.getManager().getBoundId(), "injector_FrontalDistance");
    glUniform1f(loc, context.getCameraParameters().m_frontOpticalAxisCentreDistance);

    const auto maxViews = context.getOutputFBO().getParams().getLayers();
    auto location = glGetUniformLocation(context.getManager().getBoundId(), "injector_max_views");
    glUniform1i(location, maxViews);

    location = glGetUniformLocation(context.getManager().getBoundId(), "injector_max_invocations");
    glUniform1i(location, maxViews);

    loc = glGetUniformLocation(context.getManager().getBoundId(), "injector_identity");

    bool shouldNotUseIdentity = (context.getManager().getBound()->isInjected());
    glUniform1i(loc, !shouldNotUseIdentity);
}
