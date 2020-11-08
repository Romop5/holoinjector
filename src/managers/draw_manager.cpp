#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "context.hpp"

#include "draw_manager.hpp"

#include "pipeline/projection_estimator.hpp"

using namespace ve;

void DrawManager::draw(Context& context, const std::function<void(void)>& drawCallLambda)
{
    //-------------------------------------------------------------------------
    auto drawWrapper = [&]()
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
    };
    //-------------------------------------------------------------------------
    if(!context.m_IsMultiviewActivated || (context.m_FBOTracker.hasBounded() && !context.m_FBOTracker.isSuitableForRepeating()) )
    {
        setEnhancerIdentity(context);
        drawWrapper();
        return;
    }

    if(!context.m_FBOTracker.hasBounded())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, context.m_OutputFBO.getFBOId());
        glViewport(0,0,context.m_OutputFBO.getParams().getTextureWidth(),context.m_OutputFBO.getParams().getTextureHeight());
    } else {
        auto fbo = context.m_FBOTracker.getBound();
        if(!fbo->hasShadowFBO() && context.m_FBOTracker.isSuitableForRepeating())
        {
            fbo->createShadowedFBO();
        }
        if(fbo->hasShadowFBO())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo->getShadowFBO());
        }
    }

    /// If Uniform Buffer Object is used
    if(context.m_Manager.hasBounded() && context.m_Manager.getBound()->m_Metadata && context.m_Manager.getBound()->m_Metadata->isUBOused())
    {
        const auto& blockName = context.m_Manager.getBound()->m_Metadata->m_InterfaceBlockName;
        auto index = context.m_Manager.getBound()->m_UniformBlocks[blockName].bindingIndex;
        const auto& indexStructure = context.m_UniformBlocksTracker.getBindingIndex(index);
        if(indexStructure.hasTransformation)
        {
            setEnhancerDecodedProjection(context,context.m_Manager.getBoundId(),indexStructure.projection);
        } else {
            printf("[Repeater] Unexpected state. Expected UBO, but not found. Falling back to identity\n");
            setEnhancerIdentity(context);
        }
    }
    auto loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_XShiftMultiplier");
    glUniform1f(loc, context.m_cameraParameters.m_XShiftMultiplier);

    loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_FrontalDistance");
    glUniform1f(loc, context.m_cameraParameters.m_frontOpticalAxisCentreDistance);

    auto location = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_max_views");
    glUniform1i(location, 3*3);

    location = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_max_invocations");
    glUniform1i(location, 3*3);

    //setEnhancerShift(context,glm::mat4(1.0),0.0);

    loc = glGetUniformLocation(context.m_Manager.getBoundId(), "enhancer_identity");

    bool shouldNotUseIdentity = (context.m_Manager.getBound()->m_Metadata && context.m_Manager.getBound()->m_Metadata->hasDetectedTransformation());
    glUniform1i(loc, !shouldNotUseIdentity);
    drawWrapper();
    //resetEnhancerShift(context,);
    return;
    // Get original viewport
    auto originalViewport = context.currentViewport;
    auto originalScissor = context.currentScissorArea;

    const auto setup = context.m_cameras.getCameraGridSetup();
    const auto& tilesPerX = setup.first;
    const auto& tilesPerY = setup.second;
    // for each virtual camera, create a subview (subviewport)
    // and render draw call using its transformation into this subview

    size_t cameraID = 0;
    for(const auto& camera: context.m_cameras.getCameras())
    {
        // If only-camera mode is active && ID does not match
        if(context.m_diagnostics.shouldShowOnlySpecificVirtualCamera() &&
                cameraID++ != context.m_diagnostics.getOnlyCameraID())
            continue;
        const auto& currentStartX = camera.getViewport().getX();
        const auto& currentStartY = camera.getViewport().getY();

        const auto scissorDiffX = (originalScissor.getX()-originalViewport.getX())/tilesPerX;
        const auto scissorDiffY = (originalScissor.getY()-originalViewport.getY())/tilesPerY;
        const auto scissorWidth = originalScissor.getWidth()/tilesPerX; 
        const auto scissorHeight = originalScissor.getHeight()/tilesPerY; 
        if(!context.m_diagnostics.shouldShowOnlySpecificVirtualCamera())
        {
            glScissor(currentStartX+scissorDiffX, currentStartY+scissorDiffY, scissorWidth, scissorHeight);
        }

        // Detect if VS renders into clip-space, thus if z is always 1.0
        // => in such case, we don't want to translate the virtual camera
        bool isClipSpaceRendering = false;
        if(context.m_Manager.hasBounded() && context.m_Manager.isVSBound())
        {
            isClipSpaceRendering = (context.m_Manager.getBound()->m_Metadata->m_IsClipSpaceTransform);
        }
        const auto& t = (isClipSpaceRendering)?camera.getViewMatrixRotational():camera.getViewMatrix();
        const auto& v = camera.getViewport();
        if(!context.m_diagnostics.shouldShowOnlySpecificVirtualCamera())
        {
            glViewport(v.getX(), v.getY(), v.getWidth(), v.getHeight());
        }
        setEnhancerShift(context,t,camera.getAngle()*context.m_cameraParameters.m_XShiftMultiplier/context.m_cameraParameters.m_frontOpticalAxisCentreDistance);
        drawWrapper();
        resetEnhancerShift(context);
    }
    // restore
    glViewport(originalViewport.getX(), originalViewport.getY(), originalViewport.getWidth(), originalViewport.getHeight());
    glScissor(originalScissor.getX(), originalScissor.getY(), originalScissor.getWidth(), originalScissor.getHeight());

}


void DrawManager::setEnhancerShift(Context& context,const glm::mat4& viewSpaceTransform, float projectionAdjust)
{
    const auto& resultMat = viewSpaceTransform;
    auto program = context.m_Manager.getBoundId();
    if(program)
    {
        //auto location = glGetUniformLocation(program, "enhancer_view_transform");
        //glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(glm::value_ptr(resultMat)));

        auto location = glGetUniformLocation(program, "enhancer_identity");
        glUniform1i(location, GL_FALSE);

        //location = glGetUniformLocation(program, "enhancer_projection_adjust");
        //glUniform1f(location, projectionAdjust);
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
        glMatrixMode(GL_PROJECTION);
        auto oldProjection = context.m_LegacyTracker.getProjection();
        oldProjection[2][0] = projectionAdjust;
        const auto newProjection = oldProjection*resultMat;
        if(!context.m_LegacyTracker.isOrthogonalProjection())
            glLoadMatrixf(glm::value_ptr(newProjection));
    }
}

void DrawManager::resetEnhancerShift(Context& context)
{
    if(context.m_LegacyTracker.isLegacyNeeded())
    {
        glLoadMatrixf(glm::value_ptr(context.m_LegacyTracker.getProjection()));
        glMatrixMode(context.m_LegacyTracker.getMatrixMode());
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
