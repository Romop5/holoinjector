#include "repeater.hpp"
#include <iostream>
#include <cassert>
#include <regex>
#include <unordered_set>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "FreeImage.h" // FreeImage image storing

#include "pipeline/shader_inspector.hpp"
#include "pipeline/projection_estimator.hpp"
#include "pipeline/pipeline_injector.hpp"

#include "utils/opengl_utils.hpp"
#include "utils/enviroment.hpp"
#include "utils/glsl_preprocess.hpp"

#include "logger.hpp"
#include "config.hpp"

#include <imgui.h>

using namespace ve;

void Repeater::initialize()
{
    Logger::log("Repeater::initialize\n");
    static bool isInitialzed = false;
    if(isInitialzed)
        return;
    isInitialzed = true;

    // Load config
    Config cfg;
    const auto settings = cfg.load();

    // Override default angle
    if(settings.hasKey("xmultiplier"))
        m_Context.m_cameraParameters.m_XShiftMultiplier = settings.getAsFloat("xmultiplier");
    // Override default center of rotation
    if(settings.hasKey("distance"))
        m_Context.m_cameraParameters.m_frontOpticalAxisCentreDistance = settings.getAsFloat("distance");
    // if ENHANCER_NOW is provided, then start with multiple views right now
    if(settings.hasKey("now"))
        m_Context.m_IsMultiviewActivated = true;
    if(settings.hasKey("exitAfterFrames"))
        m_Context.m_diagnostics.setTerminationAfterFrame(settings.getAsSizet("exitAfterFrames"));

    /*
     * DIAGNOSTIC
     */
    if(settings.hasKey("onlyShownCameraID"))
        m_Context.m_diagnostics.setOnlyVirtualCamera(settings.getAsSizet("onlyShownCameraID"));

    if(settings.hasKey("screenshotFormatString"))
        m_Context.m_diagnostics.setScreenshotFormat(settings.getAsString("screenshotFormatString"));

    if(settings.hasKey("shouldBeNonIntrusive"))
        m_Context.m_diagnostics.setNonIntrusiveness(true);

    // Initialize oputput FBO
    ve::pipeline::OutputFBOParameters outParameters;
    if(settings.hasKey("outputXSize"))
        outParameters.pixels_width = settings.getAsSizet("outputXSize");
    if(settings.hasKey("outputYSize"))
        outParameters.pixels_width = settings.getAsSizet("outputYSize");

    if(settings.hasKey("gridXSize"))
        outParameters.gridXSize = settings.getAsSizet("gridXSize");
    if(settings.hasKey("gridYSize"))
        outParameters.gridYSize = settings.getAsSizet("gridYSize");

    // Initialize hidden FBO for redirecting draws to back-buffer
    m_Context.m_OutputFBO.initialize(outParameters);
    assert(OpenglRedirectorBase::glGetError() == GL_NO_ERROR);

    const auto layers = m_Context.m_OutputFBO.getParams().getLayers();
    const auto gridXSize = m_Context.m_OutputFBO.getParams().getGridSizeX();
    // Fill viewports
    OpenglRedirectorBase::glGetIntegerv(GL_VIEWPORT, m_Context.currentViewport.getDataPtr());
    OpenglRedirectorBase::glGetIntegerv(GL_SCISSOR_BOX, m_Context.currentScissorArea.getDataPtr());
    // Initialize a cache of windows's subviews
    m_Context.m_cameras.setupWindows(layers, gridXSize);
    m_Context.m_cameras.updateViewports(m_Context.currentViewport);
    m_Context.m_cameras.updateParamaters(m_Context.m_cameraParameters);

    // Initialize GUI
    m_Context.m_gui.initialize();
}


void Repeater::deinitialize()
{
    // Clean up layered FBO & shaders
    m_Context.m_OutputFBO.deinitialize();
    // Clean up texture views & etc
    m_Context.m_TextureTracker.deinitialize();
}

GLint Repeater::getCurrentID(GLenum target)
{
    GLint id;
    OpenglRedirectorBase::glGetIntegerv(target, &id);
    return id;
}

void Repeater::glClear(GLbitfield mask)
{
    if(m_Context.m_IsMultiviewActivated && m_Context.m_FBOTracker.isFBODefault() &&  m_Context.m_OutputFBO.hasImage())
    {
        OpenglRedirectorBase::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        OpenglRedirectorBase::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        OpenglRedirectorBase::glViewport(m_Context.currentViewport.getX(), m_Context.currentViewport.getY(),
                    m_Context.currentViewport.getWidth(), m_Context.currentViewport.getHeight());
        m_Context.m_OutputFBO.renderToBackbuffer(m_Context.m_cameraParameters);
        m_Context.m_OutputFBO.clearBuffers();
    }

    OpenglRedirectorBase::glClear(mask);
}
void Repeater::registerCallbacks() 
{
    registerOpenGLSymbols();
}

void Repeater::glXSwapBuffers(	Display * dpy, GLXDrawable drawable)
{
    if(m_Context.m_IsMultiviewActivated && m_Context.m_OutputFBO.hasImage())
    {
        OpenglRedirectorBase::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        OpenglRedirectorBase::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        OpenglRedirectorBase::glViewport(m_Context.currentViewport.getX(), m_Context.currentViewport.getY(),
        m_Context.currentViewport.getWidth(), m_Context.currentViewport.getHeight());
        m_Context.m_OutputFBO.renderToBackbuffer(m_Context.m_cameraParameters);
        m_Context.m_OutputFBO.clearBuffers();
    } else {
        OpenglRedirectorBase::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        m_Context.m_gui.beginFrame(m_Context);
        bool shouldShow = true;
        ImGui::ShowDemoWindow(&shouldShow);
        m_Context.m_gui.endFrame();
        m_Context.m_gui.renderCurrentFrame();
    }

    OpenglRedirectorBase::glXSwapBuffers(dpy, drawable);
    m_Context.m_diagnostics.incrementFrameCount();
    if(m_Context.m_diagnostics.hasReachedLastFrame())
    {
        // Note: this is debug only, leaves mem. leaks and uncleaned objects
        takeScreenshot(std::string(m_Context.m_diagnostics.getScreenshotName()));
        exit(5);
    }
}

Bool Repeater::glXMakeCurrent(Display * dpy, GLXDrawable drawable,GLXContext context)
{
    return Repeater::glXMakeContextCurrent(dpy, drawable, drawable, context);
}

Bool Repeater::glXMakeContextCurrent(Display * dpy, GLXDrawable draw, GLXDrawable read,GLXContext context)
{
    if(draw == None && context == nullptr)
    { // release our resources
        //deinitialize();
        return OpenglRedirectorBase::glXMakeContextCurrent(dpy,draw,read,context);
    } else {
        auto result = OpenglRedirectorBase::glXMakeContextCurrent(dpy,draw,read,context);
        initialize();
        return result;
    }
}

void Repeater::glGenTextures(GLsizei n,GLuint* textures)
{
    OpenglRedirectorBase::glGenTextures(n, textures);

    for(size_t i = 0; i < n; i++)
    {
        auto texture = std::make_shared<ve::trackers::TextureMetadata>(textures[i]);
	m_Context.m_TextureTracker.add(textures[i], texture);
    }
}

void Repeater::glTexImage1D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLint border,GLenum format,GLenum type,const GLvoid* pixels) 
{
    OpenglRedirectorBase::glTexImage1D(target, level, internalFormat, width, border, format, type, pixels);
    auto finalFormat = ve::trackers::TextureTracker::isSizedFormat(internalFormat)?internalFormat:ve::trackers::TextureTracker::convertToSizedFormat(format,type);
    m_Context.m_TextureTracker.get(getCurrentID(ve::trackers::TextureTracker::getParameterForType(target)))->setStorage(target,width, 0, level, 0, finalFormat);
}
void Repeater::glTexImage2D(GLenum target,GLint level,GLint internalFormat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const GLvoid* pixels)
{
    OpenglRedirectorBase::glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    auto finalFormat = ve::trackers::TextureTracker::isSizedFormat(internalFormat)?internalFormat:ve::trackers::TextureTracker::convertToSizedFormat(format,type);
    m_Context.m_TextureTracker.get(getCurrentID(ve::trackers::TextureTracker::getParameterForType(target)))->setStorage(target,width, height, level, 0, finalFormat);
}

void Repeater::glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
    OpenglRedirectorBase::glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
    auto finalFormat = ve::trackers::TextureTracker::isSizedFormat(internalformat)?internalformat:ve::trackers::TextureTracker::convertToSizedFormat(format,type);
    m_Context.m_TextureTracker.get(getCurrentID(ve::trackers::TextureTracker::getParameterForType(target)))->setStorage(target,width, height, level, 0, finalFormat);
}

void Repeater::glTexStorage1D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    OpenglRedirectorBase::glTexStorage1D(target,levels,internalformat,width);
    m_Context.m_TextureTracker.get(getCurrentID(ve::trackers::TextureTracker::getParameterForType(target)))->setStorage(target,width, 0, levels, 0, internalformat);
}
void Repeater::glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glTexStorage2D(target,levels,internalformat,width,height);
    m_Context.m_TextureTracker.get(getCurrentID(ve::trackers::TextureTracker::getParameterForType(target)))->setStorage(target,width, height, levels, 0, internalformat);
}
void Repeater::glTexStorage3D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    OpenglRedirectorBase::glTexStorage3D(target,levels,internalformat,width,height, depth);
    m_Context.m_TextureTracker.get(getCurrentID(ve::trackers::TextureTracker::getParameterForType(target)))->setStorage(target,width, height, levels, depth, internalformat);
}

void Repeater::glTextureStorage1D (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
    OpenglRedirectorBase::glTextureStorage1D(texture,levels,internalformat,width);
    m_Context.m_TextureTracker.get(texture)->setStorage(GL_TEXTURE_1D,width, 0, levels, 0, internalformat);
}
void Repeater::glTextureStorage2D (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glTextureStorage2D(texture,levels,internalformat,width,height);
    m_Context.m_TextureTracker.get(texture)->setStorage(GL_TEXTURE_2D,width, height, levels, 0, internalformat);
}
void Repeater::glTextureStorage3D (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    OpenglRedirectorBase::glTextureStorage3D(texture,levels,internalformat,width,height,depth);
    m_Context.m_TextureTracker.get(texture)->setStorage(GL_TEXTURE_3D,width, height, levels, depth, internalformat);
}

void Repeater::glGenRenderbuffers (GLsizei n, GLuint* renderbuffers)
{
    OpenglRedirectorBase::glGenRenderbuffers(n,renderbuffers);

    for(size_t i = 0; i < n; i++)
    {
        m_Context.m_RenderbufferTracker.add(renderbuffers[i],std::make_shared<ve::trackers::RenderbufferMetadata>(renderbuffers[i]));
    }
}
void Repeater::glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glRenderbufferStorage(target,internalformat, width, height);
    // Fix internal format when transfering renderbuffer to texture
    internalformat = (internalformat == GL_DEPTH_COMPONENT?GL_DEPTH_COMPONENT32:internalformat);
    m_Context.m_RenderbufferTracker.get(getCurrentID(GL_RENDERBUFFER_BINDING))->setStorage(target,width, height, 0, 0, internalformat);
}


void Repeater::glBindTexture(GLenum target,GLuint texture)
{
    m_Context.m_TextureTracker.bind(target,texture);
    auto fakeTextureId = texture;
    if(m_Context.m_TextureTracker.has(texture) && m_Context.m_TextureTracker.get(texture)->hasShadowTexture())
    {
        fakeTextureId = m_Context.m_TextureTracker.get(texture)->getTextureViewIdOfShadowedTexture();
    }
    OpenglRedirectorBase::glBindTexture(target,fakeTextureId);
}

void Repeater::glActiveTexture (GLenum texture)
{
    OpenglRedirectorBase::glActiveTexture(texture);
    m_Context.m_TextureTracker.activate(texture-GL_TEXTURE0);
}

GLuint Repeater::glCreateShader(GLenum shaderType)
{
    auto id = OpenglRedirectorBase::glCreateShader(shaderType);
    auto shaderDesc = std::make_shared<ve::trackers::ShaderMetadata>(id,shaderType);
    m_Context.m_Manager.shaders.add(id, shaderDesc);
    return id;
}

void Repeater::glShaderSource (GLuint shaderId, GLsizei count, const GLchar* const*string, const GLint* length)
{
    auto concatenatedShader = glsl_preprocess::joinGLSLshaders(count, string, length);
    Logger::log("[Repeater] glShaderSource: [{}]\n",shaderId);
    if(m_Context.m_Manager.shaders.has(shaderId))
    {
        auto shader = m_Context.m_Manager.shaders.get(shaderId);
        auto preprocessedShader = glsl_preprocess::preprocessGLSLCode(concatenatedShader);
        shader->preprocessedSourceCode = preprocessedShader;
    }
    std::vector<const char*> shaders = {concatenatedShader.c_str(),};
    OpenglRedirectorBase::glShaderSource(shaderId,1,shaders.data(),nullptr);
}

void Repeater::glLinkProgram (GLuint programId)
{
    // Link the program for 1st time
    // => we can use native GLSL compiler to detect active uniforms
    //OpenglRedirectorBase::glLinkProgram(programId);

    // Note: this should never happen (if we handle all glCreateProgram/Shader)
    if(!m_Context.m_Manager.has(programId))
        return;

    auto program = m_Context.m_Manager.get(programId);

    /*
     *  Create pipeline injector with correct parameters (number of vies)
     */
    ve::pipeline::PipelineInjector plInjector;
    ve::pipeline::PipelineInjector::PipelineType pipeline;
    ve::pipeline::PipelineParams parameters;

    // TODO: detect if number of invocations is supported
    parameters.countOfInvocations = m_Context.m_OutputFBO.getParams().getLayers();
    if(parameters.countOfInvocations > 32)
    {
        parameters.countOfPrimitivesDuplicates = parameters.countOfInvocations/32;
        parameters.countOfInvocations = 32;
    }

    /*
     * Deregister all attached shaders (they're going to be changed in any case)
     * and fill pipeline structure out of them
     */
    for(auto [type,shader]: program->shaders.getMap())
    {
        // detach shader from program
        OpenglRedirectorBase::glDetachShader(programId, shader->m_Id);
        // store source code for given shader type
        pipeline[shader->m_Type] = shader->preprocessedSourceCode;
        Logger::log("[Repeater] Detaching: {} {}\n", shader->m_Type, shader->m_Id);
    }

    /*
     * Inject pipeline
     */
    auto resultPipeline = plInjector.process(pipeline,parameters);
    program->m_Metadata = std::move(resultPipeline.metadata);

    for(auto& [type, sourceCode]: resultPipeline.pipeline)
    {
        auto newShader = OpenglRedirectorBase::glCreateShader(type);
        const GLchar* sources[1] = {reinterpret_cast<const GLchar*>(sourceCode.data())}; 
        OpenglRedirectorBase::glShaderSource(newShader, 1, sources , nullptr);
	Logger::log("[Repeater] Compiling shader: \n {}\n", sourceCode.c_str());
        fflush(stdout);
        OpenglRedirectorBase::glCompileShader(newShader);
        GLint status;
        OpenglRedirectorBase::glGetShaderiv(newShader,GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
        {
            GLint logSize = 0;
            OpenglRedirectorBase::glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &logSize);
            
            GLsizei realLogLength = 0;
            GLchar log[5120] = {0,};
            OpenglRedirectorBase::glGetShaderInfoLog(newShader, logSize, &realLogLength, log);
            Logger::log("[Repeater] Error while compiling new shader type {}: {}\n", type,log);
            Logger::log("Shade source: {}\n", sourceCode.c_str());
            return;
        }
        OpenglRedirectorBase::glAttachShader(programId, newShader);
    }
    Logger::log("[Repeater] Relink program with new shaders\n");
    OpenglRedirectorBase::glLinkProgram(programId);
    GLint linkStatus = 0;
    OpenglRedirectorBase::glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
    {
        GLint logSize = 0;
        OpenglRedirectorBase::glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logSize);

        GLsizei realLogLength = 0;
        GLchar log[5120] = {0,};
        OpenglRedirectorBase::glGetProgramInfoLog(programId, logSize, &realLogLength, log);
        Logger::log("[Repeater] Link failed with log: {}\n",log);
    }
    assert(linkStatus == GL_TRUE);
}

void Repeater::glCompileShader (GLuint shader)
{
    Logger::log("[Repeater] glCompileShader\n");
    OpenglRedirectorBase::glCompileShader(shader);
    GLint status;
    OpenglRedirectorBase::glGetShaderiv(shader, GL_COMPILE_STATUS,&status);
    if(status == GL_FALSE)
    {
        Logger::log("[Repeater] Error while comping shader [{}]\n", shader);
    }
}

void Repeater::glAttachShader (GLuint program, GLuint shader)
{
    Logger::log("[Repeater] attaching shader [{}] to program [{}] \n", shader, program);
    //OpenglRedirectorBase::glAttachShader(program,shader);

    if(!m_Context.m_Manager.has(program) || !m_Context.m_Manager.shaders.has(shader))
        return;
    m_Context.m_Manager.get(program)->attachShaderToProgram(m_Context.m_Manager.shaders.get(shader));
}


void Repeater::glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    OpenglRedirectorBase::glUniformMatrix4fv (location, count, transpose, value);

    // get current's program transformation matrix name
    if(!m_Context.m_Manager.hasBounded())
        return;
    auto program = m_Context.m_Manager.getBound();
    if(!program->m_Metadata)
        return;
    auto metaData = program->m_Metadata.get();

    if(!metaData->hasDetectedTransformation())
        return;

    auto programID = m_Context.m_Manager.getBoundId();
    // get original MVP matrix location
    auto originalLocation = OpenglRedirectorBase::glGetUniformLocation(programID, metaData->m_TransformationMatrixName.c_str());
    
    // if the matrix being uploaded isn't detected MVP, then continue
    if(originalLocation != location)
        return;

    // estimate projection matrix from value
    const auto mat = opengl_utils::createMatrixFromRawGL(value);
    auto estimatedParameters = ve::pipeline::estimatePerspectiveProjection(mat);

    Logger::log("[Repeater] estimating parameters from uniform matrix\n");
    Logger::log("[Repeater] parameters: fx({}) fy({}) near ({}) near({}) isPerspective ({}) \n", estimatedParameters.fx, estimatedParameters.fy, estimatedParameters.nearPlane, estimatedParameters.farPlane, estimatedParameters.isPerspective);

    m_DrawManager.setEnhancerDecodedProjection(m_Context,programID, estimatedParameters);
}

GLuint Repeater::glCreateProgram (void)
{
    auto result = OpenglRedirectorBase::glCreateProgram();

    auto program = std::make_shared<ve::trackers::ShaderProgram>();
    m_Context.m_Manager.add(result, program);
    return result;
}

void Repeater::glUseProgram (GLuint program)
{
    OpenglRedirectorBase::glUseProgram(program);
    m_Context.m_Manager.bind(program);
}


void Repeater::glViewport(GLint x,GLint y,GLsizei width,GLsizei height)
{
    OpenglRedirectorBase::glViewport(x,y,width,height);
    m_Context.currentViewport.set(x,y,width,height);
    m_Context.m_cameras.updateViewports(m_Context.currentViewport);
}

void Repeater::glScissor(GLint x,GLint y,GLsizei width,GLsizei height)
{
    OpenglRedirectorBase::glScissor(x,y,width,height);
    m_Context.currentScissorArea.set(x,y,width,height);
}


//-----------------------------------------------------------------------------
// Duplicate API calls
//-----------------------------------------------------------------------------

void Repeater::glDrawArrays(GLenum mode,GLint first,GLsizei count) 
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawArrays(mode,first,count); });
}

void Repeater::glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawArraysInstanced(mode,first,count,instancecount); });
}

void Repeater::glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) 
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawElements(mode,count,type,indices); });
}

void Repeater::glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawElementsInstanced(mode,count,type,indices,instancecount); });
}

void Repeater::glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawRangeElements(mode,start,end,count,type,indices);});
}

void Repeater::glDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawElementsBaseVertex(mode,count,type,indices, basevertex); });
}
void Repeater::glDrawRangeElementsBaseVertex (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawRangeElementsBaseVertex(mode,start,end,count,type,indices,basevertex); });
}
void Repeater::glDrawElementsInstancedBaseVertex (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) 
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glDrawElementsInstancedBaseVertex(mode,count,type,indices, instancecount, basevertex); });
}

void Repeater::glMultiDrawElementsBaseVertex (GLenum mode, const GLsizei* count, GLenum type, const void* const*indices, GLsizei drawcount, const GLint* basevertex)
{
    m_DrawManager.draw(m_Context,[&]() { OpenglRedirectorBase::glMultiDrawElementsBaseVertex(mode,count,type,indices, drawcount, basevertex); });
}

// ----------------------------------------------------------------------------

void Repeater::glGenFramebuffers (GLsizei n, GLuint* framebuffers)
{
    OpenglRedirectorBase::glGenFramebuffers(n, framebuffers);
    for(size_t i=0;i < n; i++)
    {
        auto fbo = std::make_shared<ve::trackers::FramebufferMetadata>();
        m_Context.m_FBOTracker.add(framebuffers[i],fbo);
    }
}

void Repeater::glBindFramebuffer (GLenum target, GLuint framebuffer)
{
    m_Context.m_FBOTracker.bind(framebuffer);
    if(framebuffer == 0)
    {
        if(m_Context.m_IsMultiviewActivated)
        {
            OpenglRedirectorBase::glBindFramebuffer(target, m_Context.m_OutputFBO.getFBOId());
            OpenglRedirectorBase::glViewport(0,0,m_Context.m_OutputFBO.getParams().getTextureWidth(), m_Context.m_OutputFBO.getParams().getTextureHeight());
        } else {
            OpenglRedirectorBase::glBindFramebuffer(target, 0);
            OpenglRedirectorBase::glViewport(m_Context.currentViewport.getX(), m_Context.currentViewport.getY(),
                    m_Context.currentViewport.getWidth(), m_Context.currentViewport.getHeight());
        }
    } else {
        if(m_Context.m_IsMultiviewActivated)
        {
            auto id = framebuffer;
            auto fbo = m_Context.m_FBOTracker.getBound();
            /*
             * Only create & bind shadow FBO when original FBO is complete (thus has any attachment)
             */
            if(fbo->hasAnyAttachment())
            {
                if(!fbo->hasShadowFBO() && m_Context.m_FBOTracker.isSuitableForRepeating())
                    fbo->createShadowedFBO(m_Context.m_OutputFBO.getParams().getLayers());
                // Creation of shadow FBO should never fail
                assert(fbo->hasShadowFBO());
                id = fbo->getShadowFBO();
            }

            OpenglRedirectorBase::glBindFramebuffer(target,id);
            OpenglRedirectorBase::glViewport(m_Context.currentViewport.getX(), m_Context.currentViewport.getY(),
                    m_Context.currentViewport.getWidth(), m_Context.currentViewport.getHeight());

            // TODO: shadowed textures are the same size as OutputFBO
            if(m_Context.m_IsMultiviewActivated)
            {
                OpenglRedirectorBase::glViewport(0,0,m_Context.m_OutputFBO.getParams().getTextureWidth(), m_Context.m_OutputFBO.getParams().getTextureHeight());
            }
        } else {
            OpenglRedirectorBase::glBindFramebuffer(target, framebuffer);
        }
    }
}

void Repeater::glFramebufferTexture (GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture(target,attachment, texture,level);

    assert(m_Context.m_TextureTracker.has(texture));
    m_Context.m_FBOTracker.getBound()->attach(attachment, m_Context.m_TextureTracker.get(texture));
}

void Repeater::glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture1D(target,attachment,textarget, texture,level);
    m_Context.m_FBOTracker.getBound()->attach(attachment, m_Context.m_TextureTracker.get(texture),ve::trackers::FramebufferAttachment::ATTACHMENT_1D);
}
void Repeater::glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture2D(target,attachment,textarget, texture,level);
    m_Context.m_FBOTracker.getBound()->attach(attachment, m_Context.m_TextureTracker.get(texture),ve::trackers::FramebufferAttachment::ATTACHMENT_2D);
}
void Repeater::glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    OpenglRedirectorBase::glFramebufferTexture3D(target,attachment,textarget, texture,level,zoffset);
    m_Context.m_FBOTracker.getBound()->attach(attachment, m_Context.m_TextureTracker.get(texture),ve::trackers::FramebufferAttachment::ATTACHMENT_3D);
}

void Repeater::glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    OpenglRedirectorBase::glFramebufferRenderbuffer(target,attachment,renderbuffertarget, renderbuffer);
    m_Context.m_FBOTracker.getBound()->attach(attachment, m_Context.m_RenderbufferTracker.get(renderbuffer),ve::trackers::FramebufferAttachment::ATTACHMENT_2D);
}
// ----------------------------------------------------------------------------
GLuint Repeater::glGetUniformBlockIndex (GLuint program, const GLchar* uniformBlockName)
{
    auto result = OpenglRedirectorBase::glGetUniformBlockIndex(program, uniformBlockName);
    if(!m_Context.m_Manager.has(program))
        return result;
    auto record = m_Context.m_Manager.get(program);
    if(record->m_UniformBlocks.count(uniformBlockName) == 0)
    {
        ve::trackers::ShaderProgram::UniformBlock block;
        block.location = result;
        record->m_UniformBlocks[std::string(uniformBlockName)] = block;
    }
    return result;
}

void Repeater::glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) 
{
    OpenglRedirectorBase::glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    if(!m_Context.m_Manager.has(program))
        return;

    auto record = m_Context.m_Manager.get(program);
    record->updateUniformBlock(uniformBlockIndex, uniformBlockBinding);

    // Add binding index's transformation metadata
    if(record->m_Metadata)
    {
        const auto desc = record->m_Metadata.get();
        if(desc->isUBOused() && record->hasUniformBlock(desc->m_InterfaceBlockName))
        {
            auto& block = record->m_UniformBlocks[desc->m_InterfaceBlockName];
            if(OpenglRedirectorBase::glGetUniformBlockIndex(program,desc->m_InterfaceBlockName.c_str())  == uniformBlockIndex)
            {
                auto& index = m_Context.m_UniformBlocksTracker.getBindingIndex(block.bindingIndex);

                std::array<const GLchar*, 1> uniformList = {desc->m_TransformationMatrixName.c_str()};
                std::array<GLuint, 1> resultIndex;
                std::array<GLint, 1> params;
                OpenglRedirectorBase::glGetUniformIndices(program, 1, uniformList.data(), resultIndex.data());
                OpenglRedirectorBase::glGetActiveUniformsiv(program, 1, resultIndex.data(), GL_UNIFORM_OFFSET, params.data());

                // Store offset of uniform in block
                index.transformationOffset = params[0];
            }
        }
    }
}

void Repeater::glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    OpenglRedirectorBase::glBindBufferRange(target, index, buffer, offset, size);
    if(target == GL_UNIFORM_BUFFER)
        m_Context.m_UniformBlocksTracker.setUniformBinding(buffer,index);
}
void Repeater::glBindBufferBase (GLenum target, GLuint index, GLuint buffer)
{
    OpenglRedirectorBase::glBindBufferBase(target, index, buffer);
    if(target == GL_UNIFORM_BUFFER)
        m_Context.m_UniformBlocksTracker.setUniformBinding(buffer,index);
}

void Repeater::glBindBuffersBase (GLenum target, GLuint first, GLsizei count, const GLuint* buffers)
{
    OpenglRedirectorBase::glBindBuffersBase(target,first, count, buffers);

    if(target == GL_UNIFORM_BUFFER)
    {
        for(size_t i = 0; i < count; i++)
        {
            m_Context.m_UniformBlocksTracker.setUniformBinding(buffers[i],first+i);
        }
    }
}

void Repeater::glBindBuffersRange (GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes) 
{
    OpenglRedirectorBase::glBindBuffersRange(target,first,count, buffers, offsets, sizes);

    if(target == GL_UNIFORM_BUFFER)
    {
        for(size_t i = 0; i < count; i++)
        {
            m_Context.m_UniformBlocksTracker.setUniformBinding(buffers[i],first+i);
        }
    }
}


void Repeater::glBufferData (GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    OpenglRedirectorBase::glBufferData(target,size,data,usage);

    if(target != GL_UNIFORM_BUFFER)
        return;

    GLint bufferID = getCurrentID(GL_UNIFORM_BUFFER_BINDING);
    if(!m_Context.m_UniformBlocksTracker.hasBufferBindingIndex(bufferID))
        return;
    auto index = m_Context.m_UniformBlocksTracker.getBufferBindingIndex(bufferID);
    auto& metadata = m_Context.m_UniformBlocksTracker.getBindingIndex(index);
    if(metadata.transformationOffset == -1)
    {
        // Find a program whose uniform iterface contains transformation matrix
        for(auto& [programID, program]: m_Context.m_Manager.getMap())
        {
            auto& blocks = program->m_UniformBlocks;
            
            // Determine fi shader program contains a block, whose binding index is same as
            // currently bound GL_UNIFORM_BUFFER
            auto result = std::find_if(blocks.begin(), blocks.end(), [&](const auto& block)->bool
            {
                return (block.second.bindingIndex == index);
            });
            // If index is 0, than any block would do
            if(result == blocks.end() && index != 0)
                continue;
            // Determine if program's VS has transformation in interface block
            if(!program->m_Metadata)
                continue;
            const auto programMetadata = program->m_Metadata.get();
            if(!programMetadata->isUBOused())
                continue;

            std::array<const GLchar*, 1> uniformList = {programMetadata->m_TransformationMatrixName.c_str()};
            std::array<GLuint, 1> resultIndex;
            std::array<GLint, 1> params;
            OpenglRedirectorBase::glGetUniformIndices(programID, 1, uniformList.data(), resultIndex.data());
            OpenglRedirectorBase::glGetActiveUniformsiv(programID, 1, resultIndex.data(), GL_UNIFORM_OFFSET, params.data());

            // Store offset of uniform in block
            metadata.transformationOffset = params[0];

            // Re-store bindingIndex if needed
            blocks[programMetadata->m_InterfaceBlockName].bindingIndex = index;
        }
    }
    if(metadata.transformationOffset != -1)
    {
        if(size >= metadata.transformationOffset+sizeof(float)*16)
        {
            std::memcpy(glm::value_ptr(metadata.transformation), static_cast<const std::byte*>(data)+metadata.transformationOffset, sizeof(float)*16);
            auto estimatedParameters = ve::pipeline::estimatePerspectiveProjection(metadata.transformation);

            Logger::log("[Repeater] estimating parameters from UBO\n");
            Logger::log("[Repeater] parameters: fx({}) fy({}) near ({}) near({}) isPerspective ({}) \n", estimatedParameters.fx, estimatedParameters.fy, estimatedParameters.nearPlane, estimatedParameters.farPlane, estimatedParameters.isPerspective);

            // TODO: refactor into class method of Binding index structure
            metadata.projection = estimatedParameters;
            metadata.hasTransformation = true;
        }
    }
}
void Repeater::glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    OpenglRedirectorBase::glBufferSubData(target,offset,size,data);

    if(target != GL_UNIFORM_BUFFER)
        return;

    GLint bufferID = 0;
    OpenglRedirectorBase::glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &bufferID);
    if(!m_Context.m_UniformBlocksTracker.hasBufferBindingIndex(bufferID))
        return;
    auto index = m_Context.m_UniformBlocksTracker.getBufferBindingIndex(bufferID);
    auto& metadata = m_Context.m_UniformBlocksTracker.getBindingIndex(index);
    if(metadata.transformationOffset != -1 && offset <= metadata.transformationOffset)
    {
        if(offset+size >= metadata.transformationOffset+sizeof(float)*16)
        {
            std::memcpy(glm::value_ptr(metadata.transformation), static_cast<const std::byte*>(data)+metadata.transformationOffset, sizeof(float)*16);
            auto estimatedParameters = ve::pipeline::estimatePerspectiveProjection(metadata.transformation);
            Logger::log("[Repeater] estimating parameters from UBO\n");
            Logger::log("[Repeater] parameters: fx({}) fy({}) near ({}) near({}) \n", estimatedParameters.fx, estimatedParameters.fy, estimatedParameters.nearPlane, estimatedParameters.farPlane);

            metadata.projection = estimatedParameters;
            metadata.hasTransformation = true;
        }
    }
}

// ----------------------------------------------------------------------------
void Repeater::glMatrixMode(GLenum mode)
{
    OpenglRedirectorBase::glMatrixMode(mode);
    m_Context.m_LegacyTracker.matrixMode(mode);
    //Logger::log("[Repeater] glMatrixMode {}\n", ve::opengl_utils::getEnumStringRepresentation(mode).c_str());
}
void Repeater::glLoadMatrixd(const GLdouble* m)
{
    OpenglRedirectorBase::glLoadMatrixd(m);
    if(m_Context.m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.m_LegacyTracker.loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}
void Repeater::glLoadMatrixf(const GLfloat* m)
{
    OpenglRedirectorBase::glLoadMatrixf(m);
    if(m_Context.m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.m_LegacyTracker.loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}

void Repeater::glLoadIdentity(void)
{
    OpenglRedirectorBase::glLoadIdentity();
    if(m_Context.m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        glm::mat4 identity = glm::mat4(1.0);
        m_Context.m_LegacyTracker.loadMatrix(std::move(identity));
    }
}

void Repeater::glMultMatrixd(const GLdouble* m)
{
    OpenglRedirectorBase::glMultMatrixd(m);
    if(m_Context.m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.m_LegacyTracker.loadMatrix(std::move(result));
    }
}

void Repeater::glMultMatrixf(const GLfloat* m)
{
    OpenglRedirectorBase::glMultMatrixf(m);
    if(m_Context.m_LegacyTracker.getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.m_LegacyTracker.loadMatrix(std::move(result));
    }
}


void Repeater::glOrtho(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val)
{
    OpenglRedirectorBase::glOrtho(left,right,bottom,top,near_val,far_val);
    m_Context.m_LegacyTracker.multMatrix(glm::ortho(left,right,bottom,top,near_val,far_val));
}

void Repeater::glFrustum(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val)
{
    OpenglRedirectorBase::glFrustum(left,right,bottom,top,near_val,far_val);
    m_Context.m_LegacyTracker.multMatrix(glm::frustum(left,right,bottom,top,near_val,far_val));
}

void Repeater::glBegin(GLenum mode)
{
    if(m_Context.m_callList == 0)
    {
        m_Context.m_callList = OpenglRedirectorBase::glGenLists(1);
    }
    OpenglRedirectorBase::glNewList(m_Context.m_callList, GL_COMPILE);
    OpenglRedirectorBase::glBegin(mode);
}

void Repeater::glEnd()
{
    OpenglRedirectorBase::glEnd();
    OpenglRedirectorBase::glEndList();

    m_DrawManager.draw(m_Context,[&]() {
        OpenglRedirectorBase::glCallList(m_Context.m_callList);
    });
}

void Repeater::glCallList(GLuint list)
{
    m_DrawManager.draw(m_Context,[&]() {
        OpenglRedirectorBase::glCallList(list);
    });
}
void Repeater::glCallLists(GLsizei n,GLenum type,const GLvoid* lists)
{
    m_DrawManager.draw(m_Context,[&]() {
        OpenglRedirectorBase::glCallLists(n,type, lists);
    });
}

//-----------------------------------------------------------------------------
// Debugging utils
//-----------------------------------------------------------------------------
int Repeater::XNextEvent(Display *display, XEvent *event_return)
{
    auto fillEmptyEvent = [](XEvent* event)
    {
        auto msg = reinterpret_cast<XClientMessageEvent*>(event);
        msg->type = ClientMessage;
        msg->message_type = None;
        return;
    };
    auto returnVal = OpenglRedirectorBase::XNextEvent(display,event_return);

    if(event_return->type == KeyPress || event_return->type == KeyRelease)
    {
        auto keyEvent = reinterpret_cast<XKeyEvent*>(event_return);
        static unsigned long lastSerial = 0;
        if(keyEvent->serial == lastSerial)
            return returnVal;
        lastSerial = keyEvent->serial;
        Logger::log("[Repeater] XNextEvent KeyPress {}, {} - {} - {} - [{} {}] [{} {}] {}\n",
                keyEvent->type,keyEvent->serial, keyEvent->send_event,
                static_cast<void*>(keyEvent->display),
                keyEvent->x,keyEvent->y,keyEvent->state,keyEvent->keycode, keyEvent->same_screen);
        auto keySym = XLookupKeysym(reinterpret_cast<XKeyEvent*>(event_return), 0);

        if(event_return->type == KeyPress)
            onKeyPress(keySym);
        m_Context.m_gui.onKey(keySym,(event_return->type == KeyPress));
        fillEmptyEvent(event_return);
    }

    if(event_return->type == MotionNotify)
    {
        auto mouseEvent = reinterpret_cast<XMotionEvent*>(event_return);
        auto dx = mouseEvent->x-X11MouseHook.m_LastXPosition;
        auto dy = mouseEvent->y-X11MouseHook.m_LastYPosition;
        std::swap(X11MouseHook.m_LastXPosition,mouseEvent->x);
        std::swap(X11MouseHook.m_LastYPosition,mouseEvent->y);
        m_Context.m_gui.onMousePosition(dx,dy);
        Logger::log("[Repeater] {} {}\n", dx, dy);

        fillEmptyEvent(event_return);
        return 0;
    }

    if(event_return->type == ButtonPress || event_return->type == ButtonRelease)
    {
        const auto isPressed = (event_return->type == ButtonPress);
        auto event = reinterpret_cast<XButtonPressedEvent*>(event_return);
        m_Context.m_gui.onButton(event->button-1, isPressed);
    }

    return returnVal;
}

int Repeater::XWarpPointer(Display* display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y)
{
    return 0;
    X11MouseHook.m_LastXPosition = dest_x;
    X11MouseHook.m_LastYPosition = dest_y;
    return OpenglRedirectorBase::XWarpPointer(display, src_w, dest_w, src_x, src_y, src_width, src_height, dest_x, dest_y);
}

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

void Repeater::takeScreenshot(const std::string filename)
{
    const auto width = m_Context.currentViewport.getWidth();
    const auto height = m_Context.currentViewport.getHeight();
    // Make the BYTE array, factor of 3 because it's RBG.
    BYTE* pixels = new BYTE[3 * width * height];

    OpenglRedirectorBase::glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

    // Convert to FreeImage format & save to file
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0xFF, 0xFF00, 0xFF0000, false);
    if(!FreeImage_Save(FIF_BMP, image, filename.c_str(), 0))
    {
        Logger::log("[Repeater] Failed to save screenshot {}\n", filename.c_str());
    }
    // Free resources
    FreeImage_Unload(image);
    delete [] pixels;
}

void Repeater::onKeyPress(size_t keySym)
{
        const float increment = 0.10f;
        switch(keySym)
        {
            case XK_F1: case XK_F2:
                m_Context.m_cameraParameters.m_XShiftMultiplier += (keySym == XK_F1?1.0:-1.0)*increment;
            break;
            case XK_F3: case XK_F4:
                m_Context.m_cameraParameters.m_frontOpticalAxisCentreDistance += (keySym == XK_F3?1.0:-1.0)*0.5;
            break; 
            case XK_F5:
                m_Context.m_cameraParameters = ve::pipeline::CameraParameters();
            break;
            case XK_F12: case XK_F11:
                m_Context.m_IsMultiviewActivated = !m_Context.m_IsMultiviewActivated;
            default:
            break;
        }
        Logger::log("[Repeater] Setting: frontDistance ({}), X multiplier({})\n",
                m_Context.m_cameraParameters.m_frontOpticalAxisCentreDistance, m_Context.m_cameraParameters.m_XShiftMultiplier);
        m_Context.m_cameras.updateParamaters(m_Context.m_cameraParameters);
}
