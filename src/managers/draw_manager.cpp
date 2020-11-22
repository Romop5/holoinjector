#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "context.hpp"
#include "draw_manager.hpp"
#include "pipeline/projection_estimator.hpp"
#include "logger.hpp"

using namespace ve;

void DrawManager::draw(Context& context, const std::function<void(void)>& drawCallLambda)
{
   if(!context.m_IsMultiviewActivated || (context.m_FBOTracker.hasBounded() && !context.m_FBOTracker.isSuitableForRepeating()) )
    {
        setEnhancerIdentity(context);
        drawGeneric(context,drawCallLambda);
        return;
    }

    /*
     * If applications is drawing into offscreen FBO, then bind our shadowed FBO
     */
    if(!context.m_FBOTracker.hasBounded())
    {
        assert(context.m_OutputFBO.getFBOId() != 0 && "OutputFBO must be initialized");
        context.m_OutputFBO.setContainsImageFlag();
        glBindFramebuffer(GL_FRAMEBUFFER, context.m_OutputFBO.getFBOId());
        glViewport(0,0,context.m_OutputFBO.getParams().getTextureWidth(),context.m_OutputFBO.getParams().getTextureHeight());
    } else {
        auto fbo = context.m_FBOTracker.getBound();
        if(context.m_FBOTracker.isSuitableForRepeating())
        {
            // should be available since binding
            assert(fbo->hasShadowFBO());
            glBindFramebuffer(GL_FRAMEBUFFER, fbo->getShadowFBO());
        }
    }

    /// If Uniform Buffer Object (UBO) is used, then load values to uniforms
    if(context.m_Manager.hasBounded() && context.m_Manager.getBound()->m_Metadata && context.m_Manager.getBound()->m_Metadata->isUBOused())
    {
        const auto& blockName = context.m_Manager.getBound()->m_Metadata->m_InterfaceBlockName;
        auto index = context.m_Manager.getBound()->m_UniformBlocks[blockName].bindingIndex;
        const auto& indexStructure = context.m_UniformBlocksTracker.getBindingIndex(index);
        if(indexStructure.hasTransformation)
        {
            setEnhancerDecodedProjection(context,context.m_Manager.getBoundId(),indexStructure.projection);
        } else {
            Logger::log("[Repeater] Unexpected state. Expected UBO, but not found. Falling back to identity\n");
            setEnhancerIdentity(context);
        }
    }
    /*
     *  Load enhancer's values into uniforms
     */
    if(context.m_Manager.hasBounded())
    {
        auto loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_XShiftMultiplier");
        glUniform1f(loc, context.m_cameraParameters.m_XShiftMultiplier);

        loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_FrontalDistance");
        glUniform1f(loc, context.m_cameraParameters.m_frontOpticalAxisCentreDistance);

        const auto maxViews = context.m_OutputFBO.getParams().getLayers();
        auto location = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_max_views");
        glUniform1i(location, maxViews);

        location = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_max_invocations");
        glUniform1i(location, maxViews);

        loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_identity");

        bool shouldNotUseIdentity = (context.m_Manager.getBound()->m_Metadata && context.m_Manager.getBound()->m_Metadata->hasDetectedTransformation());
        glUniform1i(loc, !shouldNotUseIdentity);
    }
    drawGeneric(context,drawCallLambda);
    return;
}

void DrawManager::drawGeneric(Context& context, const std::function<void(void)>& drawCallLambda)
{
    const auto& programs = context.m_Manager;
    /// If application is using shaders and shader program has enhancer's GS capabilities
    if(programs.hasBounded() && programs.getBoundConst()->m_Metadata)
    {
        drawWithGeometryShader(context, drawCallLambda);
    } else {
        drawLegacy(context, drawCallLambda);
    }
}
void DrawManager::drawWithGeometryShader(Context& context, const std::function<void(void)>& drawCallLambda)
{
    if(!context.m_TextureTracker.getTextureUnits().hasShadowedTextureBinded())
    {
        auto loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_isSingleViewActivated");
        glUniform1i(loc, false);
        drawCallLambda();
        return;
    }

    const auto numOfLayers = context.m_OutputFBO.getParams().getLayers();
    for(size_t l = 0; l < numOfLayers; l++)
    {
        context.m_TextureTracker.getTextureUnits().bindShadowedTexturesToLayer(l);

        auto loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_isSingleViewActivated");
        glUniform1i(loc, true);

        loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_singleViewID");
        glUniform1i(loc, l);
        drawCallLambda();
    }
}

void DrawManager::drawLegacy(Context& context, const std::function<void(void)>& drawCallLambda)
{
    if(!context.m_IsMultiviewActivated || (context.m_FBOTracker.hasBounded() && !context.m_FBOTracker.isSuitableForRepeating()) )
    {
        drawCallLambda();
        return;
    }

    for(size_t cameraID = 0; cameraID < context.m_cameras.getCameras().size(); cameraID++)
    {
        const auto& camera = context.m_cameras.getCameras()[cameraID];

        auto shadowFBO = createSingleViewFBO(context, cameraID);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

        const auto& t = camera.getViewMatrix();
        setEnhancerShift(context,t,camera.getAngle()*context.m_cameraParameters.m_XShiftMultiplier/context.m_cameraParameters.m_frontOpticalAxisCentreDistance);
        drawCallLambda();
        resetEnhancerShift(context);
    }
}

/*
 * ----------------------------------------------------------------------------
 */

void DrawManager::setEnhancerShift(Context& context,const glm::mat4& viewSpaceTransform, float projectionAdjust)
{
    const auto& resultMat = viewSpaceTransform;
    auto program = context.m_Manager.getBoundId();
    if(program)
    {
        auto location = glGetUniformLocation(program, "enhancer_identity");
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

    if(context.m_LegacyTracker.isLegacyNeeded())
    {
        pushFixedPipelineProjection(context, resultMat);
    }
}

void DrawManager::resetEnhancerShift(Context& context)
{
    if(context.m_LegacyTracker.isLegacyNeeded())
    {
        popFixedPipelineProjection(context);
    }
}

void DrawManager::setEnhancerIdentity(Context& context)
{
    const auto identity = glm::mat4(1.0);
    auto program = context.m_Manager.getBoundId();
    auto location = glGetUniformLocation(program, "enhancer_identity");
    glUniform1i(location, GL_TRUE);

    location = glGetUniformLocation(program, "enhancer_max_views");
    glUniform1i(location, 1);
    location = glGetUniformLocation(program, "enhancer_max_invocations");
    glUniform1i(location, 1);
}

void DrawManager::setEnhancerDecodedProjection(Context& context, GLuint program, const PerspectiveProjectionParameters& projection)
{
    // upload parameters to GPU's program
    auto parametersLocation = glGetUniformLocation(program, "enhancer_deprojection");
    glUniform4fv(parametersLocation,1,glm::value_ptr(projection.asVector()));

    parametersLocation = glGetUniformLocation(program, "enhancer_deprojection_inv");
    glm::vec4 inverted = glm::vec4(1.0)/projection.asVector();
    glUniform4fv(parametersLocation,1,glm::value_ptr(inverted));

    auto typeLocation= glGetUniformLocation(program, "enhancer_isOrthogonal");
    glUniform1i(typeLocation,!projection.isPerspective);

}

void DrawManager::pushFixedPipelineProjection(Context& context, const glm::mat4& viewSpaceTransform, float projectionAdjust)
{
    glMatrixMode(GL_PROJECTION);
    auto oldProjection = context.m_LegacyTracker.getProjection();
    oldProjection[2][0] = projectionAdjust;
    const auto newProjection = oldProjection*viewSpaceTransform;
    if(!context.m_LegacyTracker.isOrthogonalProjection())
        glLoadMatrixf(glm::value_ptr(newProjection));
}
void DrawManager::popFixedPipelineProjection(Context& context)
{
    glLoadMatrixf(glm::value_ptr(context.m_LegacyTracker.getProjection()));
    glMatrixMode(context.m_LegacyTracker.getMatrixMode());
}

GLuint DrawManager::createSingleViewFBO(Context& context, size_t layer)
{
    if(context.m_FBOTracker.hasBounded())
    {
        return context.m_FBOTracker.getBound()->createProxyFBO(layer);
    } else {
        return context.m_OutputFBO.createProxyFBO(layer);
    }
}
