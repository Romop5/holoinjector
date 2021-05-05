/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        dispatcher.cpp
*
*****************************************************************************/

#include "dispatcher.hpp"
#include <cassert>
#include <iostream>
#include <regex>
#include <sstream>
#include <unordered_set>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "pipeline/camera_parameters.hpp"
#include "pipeline/output_fbo.hpp"
#include "pipeline/projection_estimator.hpp"
#include "pipeline/shader_inspector.hpp"
#include "pipeline/virtual_cameras.hpp"

#include "trackers/framebuffer_tracker.hpp"
#include "trackers/legacy_tracker.hpp"
#include "trackers/renderbuffer_tracker.hpp"
#include "trackers/shader_tracker.hpp"
#include "trackers/uniform_block_tracing.hpp"

#include "ui/x11_sniffer.hpp"

#include "imgui_adapter.hpp"

#include "utils/enviroment.hpp"
#include "utils/opengl_state.hpp"
#include "utils/opengl_utils.hpp"

#include "config.hpp"
#include "diagnostics.hpp"
#include "logger.hpp"

#include <imgui.h>

#include <X11/Xlib.h>

using namespace hi;

void Dispatcher::initialize()
{
    m_IsInitialized = true;

    Logger::log("Dispatcher::initialize");

    // Load config
    Config cfg;
    const auto settings = cfg.load();

    // Override default angle
    if (settings.hasKey("xmultiplier"))
    {
        m_Context.getCameraParameters().m_XShiftMultiplier = settings.getAsFloat("xmultiplier");
    }
    // Override default center of rotation
    if (settings.hasKey("distance"))
    {
        m_Context.getCameraParameters().m_frontOpticalAxisCentreDistance = settings.getAsFloat("distance");
    }
    // if HI_NOW is provided, then start with multiple views right now
    if (settings.hasKey("now"))
    {
        m_Context.m_IsMultiviewActivated = true;
    }

    // Preset shift/near-plane position for quilt
    if (settings.hasKey("wide"))
    {
        m_Context.m_IsMultiviewActivated = true;
        m_Context.getCameraParameters().m_XShiftMultiplier = 4.0;
        m_Context.getCameraParameters().m_frontOpticalAxisCentreDistance = 4.0;
    }

    // Prevent application displaying Window (useful for scripts)
    if (settings.hasKey("runInBackground"))
    {
        m_Context.keepWindowInBackgroundFlag = true;
    }

    if (settings.hasKey("quilt"))
    {
        m_Context.getOutputFBO().toggleGridView();
    }

    if (settings.hasKey("exitAfterFrames"))
    {
        m_Context.getDiagnostics().setTerminationAfterFrame(settings.getAsSizet("exitAfterFrames"));
    }

    /*
     * DIAGNOSTIC
     */
    if (settings.hasKey("onlyShownCameraID"))
    {
        m_Context.getDiagnostics().setOnlyVirtualCamera(settings.getAsSizet("onlyShownCameraID"));
    }

    if (settings.hasKey("screenshotFormatString"))
    {
        m_Context.getDiagnostics().setScreenshotFormat(settings.getAsString("screenshotFormatString"));
    }

    if (settings.hasKey("shouldBeNonIntrusive"))
    {
        m_Context.getDiagnostics().setNonIntrusiveness(true);
    }

    if (settings.hasKey("shouldRecordFPS"))
    {
        m_Context.getDiagnostics().setFPSMeasuringState(true);
    }

    // Initialize oputput FBO
    hi::pipeline::OutputFBOParameters outParameters;
    if (settings.hasKey("outputXSize"))
    {
        outParameters.pixels_width = settings.getAsSizet("outputXSize");
    }

    if (settings.hasKey("outputYSize"))
    {
        outParameters.pixels_width = settings.getAsSizet("outputYSize");
    }

    if (settings.hasKey("gridXSize"))
    {
        outParameters.gridXSize = settings.getAsSizet("gridXSize");
    }

    if (settings.hasKey("gridYSize"))
    {
        outParameters.gridYSize = settings.getAsSizet("gridYSize");
    }

    if (settings.hasKey("noGeometryShader"))
    {
        m_Context.dontInsertGeometryShader = true;
    }

    // Initialize hidden FBO for redirecting draws to back-buffer
    m_Context.getOutputFBO().initialize(outParameters);
    assert(OpenglRedirectorBase::glGetError() == GL_NO_ERROR);

    const auto layers = m_Context.getOutputFBO().getParams().getLayers();
    const auto gridXSize = m_Context.getOutputFBO().getParams().getGridSizeX();
    // Fill viewports
    OpenglRedirectorBase::glGetIntegerv(GL_VIEWPORT, m_Context.getCurrentViewport().getDataPtr());
    OpenglRedirectorBase::glGetIntegerv(GL_SCISSOR_BOX, m_Context.getCurrentScissorArea().getDataPtr());
    // Initialize a cache of windows's subviews
    m_Context.getCameras().setupWindows(layers, gridXSize);
    m_Context.getCameras().updateViewports(m_Context.getCurrentViewport());
    m_Context.getCameras().updateParamaters(m_Context.getCameraParameters());

    // Initialize GUI
    m_Context.getGui().initialize();

    /* 
     * Register input callbacks 
     */
    m_Context.getX11Sniffer().registerOnKeyCallback([&](size_t keySym, bool isDown) -> bool {
        bool shouldBlockInput = false;
        if (isDown)
        {
            m_UIManager.onKeyPressed(m_Context, keySym);
        }
        // If GUI is active, propagate input to GUI and block
        if (m_Context.getGui().isVisible())
        {
            m_Context.getGui().onKey(keySym, isDown);
            shouldBlockInput = true;
        }
        return shouldBlockInput;
    });
    m_Context.getX11Sniffer().registerOnMouseMoveCallback([&](float dx, float dy) {
        if (m_Context.getGui().isVisible())
        {
            m_Context.getGui().onMousePosition(dx, dy);
            return true;
        }
        return false;
    });

    m_Context.getX11Sniffer().registerOnButtonCallback([&](size_t buttonID, bool isPressed) {
        if (m_Context.getGui().isVisible())
        {
            m_Context.getGui().onButton(buttonID, isPressed);
            return true;
        }
        return false;
    });

    m_UIManager.initialize(m_Context);

    Logger::log("Initialized with settings: ", settings.toString());
}

void Dispatcher::deinitialize()
{
    // Clean up layered FBO & shaders
    m_Context.getOutputFBO().deinitialize();
    // Clean up texture views & etc
    m_Context.getTextureTracker().deinitialize();

    m_UIManager.deinitialize(m_Context);
    m_Context.getGui().destroy();

    m_Context.reset();
    m_IsInitialized = false;
}

GLint Dispatcher::getCurrentID(GLenum target)
{
    GLint id;
    OpenglRedirectorBase::glGetIntegerv(target, &id);
    return id;
}

void Dispatcher::glClear(GLbitfield mask)
{
    m_FramebufferManager.clear(m_Context, mask);
}

void Dispatcher::registerCallbacks()
{
    registerOpenGLSymbols();
}

GLXContext Dispatcher::glXCreateContext(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct)
{
    // TODO: code below does not work as expected in practices
    // Hint: XVisualInfo needs to be converted into corresponding visual_attribs
    return OpenglRedirectorBase::glXCreateContext(dpy, vis, shareList, direct);

    /*
     * See https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
     */
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

    glXCreateContextAttribsARBProc proc = reinterpret_cast<glXCreateContextAttribsARBProc>(glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")));
    // FBConfigs were added in GLX version 1.3.
    int glx_major, glx_minor;
    if (!(!glXQueryVersion(dpy, &glx_major, &glx_minor) || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) && proc != nullptr)
    {
        static int visual_attribs[] = {
            GLX_X_RENDERABLE, True,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER, True,
            //GLX_SAMPLE_BUFFERS  , 1,
            //GLX_SAMPLES         , 4,
            None
        };
        int fbcount;
        GLXFBConfig* fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount);
        if (fbcount)
        {
            // TODO: choose best config
            GLXFBConfig bestFbc = fbc[0];
            XFree(fbc);

            /* Use OpenGL 4.6*/
            int attrib_list[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 3,
                None
            };

            auto ctx = proc(dpy, bestFbc, shareList, direct, attrib_list);
            //if(ctx)
            //    return ctx;
            Logger::log("Failed to create OpenGL 4.6 context, falling back to original glXCreateContext");
        }
    }
    return OpenglRedirectorBase::glXCreateContext(dpy, vis, shareList, direct);
}

void Dispatcher::glXSwapBuffers(Display* dpy, GLXDrawable drawable)
{
    // Render content of OutputFBO
    m_FramebufferManager.renderFromOutputFBO(m_Context);
    // Render overlay
    {
        // Draw GUI overlay if GUI is visible
        if (m_Context.getGui().isVisible())
        {
            m_Context.getGui().beginFrame(m_Context);
            m_UIManager.onDraw(m_Context);
            m_Context.getGui().endFrame();
            m_Context.getGui().renderCurrentFrame();
        }
    }

    // Update diagnosis
    {
        if (m_Context.getDiagnostics().hasReachedLastFrame())
        {
            // Note: this is debug only, leaves mem. leaks and uncleaned objects
            const auto& viewport = m_Context.getCurrentViewport();
            opengl_utils::takeScreenshot(std::string(m_Context.getDiagnostics().getScreenshotName()), viewport.getWidth(), viewport.getHeight());
            exit(5);
        }
        m_Context.getDiagnostics().incrementFrameCount();
        Logger::getInstance().incrementFrameNumber();
    }

    // Swap buffers
    m_FramebufferManager.swapBuffers(m_Context,
        [&]() {
            ::glXSwapBuffers(dpy, drawable);
        });
}

Bool Dispatcher::glXMakeCurrent(Display* dpy, GLXDrawable drawable, GLXContext context)
{
    return Dispatcher::glXMakeContextCurrent(dpy, drawable, drawable, context);
}

Bool Dispatcher::glXMakeContextCurrent(Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext context)
{
    if (m_IsInitialized)
    {
        Logger::log("Deinitializing for context: ", static_cast<void*>(context));
        deinitialize();
    }

    if (draw == None && context == nullptr)
    {
        return OpenglRedirectorBase::glXMakeContextCurrent(dpy, draw, read, context);
    }
    else
    {
        auto result = OpenglRedirectorBase::glXMakeContextCurrent(dpy, draw, read, context);
        glGetError();
        initialize();
        return result;
    }
}

void Dispatcher::glGenTextures(GLsizei n, GLuint* textures)
{
    OpenglRedirectorBase::glGenTextures(n, textures);

    for (size_t i = 0; i < n; i++)
    {
        auto texture = std::make_shared<hi::trackers::TextureMetadata>(textures[i]);
        m_Context.getTextureTracker().add(textures[i], texture);
    }
}

void Dispatcher::glDeleteTextures(GLsizei n, const GLuint* textures)
{
    OpenglRedirectorBase::glDeleteTextures(n, textures);

    for (size_t i = 0; i < n; i++)
    {
        m_Context.getTextureTracker().remove(textures[i]);
    }
}

void Dispatcher::glTexImage1D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    OpenglRedirectorBase::glTexImage1D(target, level, internalFormat, width, border, format, type, pixels);
    auto finalFormat = hi::trackers::TextureTracker::isSizedFormat(internalFormat) ? hi::trackers::TextureTracker::convertToSizedFormat(format, type) : internalFormat;
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, 0, level, 0, finalFormat);
}
void Dispatcher::glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    OpenglRedirectorBase::glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    auto finalFormat = hi::trackers::TextureTracker::isSizedFormat(internalFormat) ? hi::trackers::TextureTracker::convertToSizedFormat(format, type) : internalFormat;
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, height, level, 0, finalFormat);
}

void Dispatcher::glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
    OpenglRedirectorBase::glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
    auto finalFormat = hi::trackers::TextureTracker::isSizedFormat(internalformat) ? hi::trackers::TextureTracker::convertToSizedFormat(format, type) : internalformat;
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, height, level, 0, finalFormat);
}

void Dispatcher::glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels)
{
    OpenglRedirectorBase::glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
    if (xoffset > 0)
    {
        Logger::logDebug("Skipping setStorage() due to xoffset != 0");
        return;
    }
    auto finalFormat = hi::trackers::TextureTracker::convertToSizedFormat(format, type);
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, 0, level, 0, finalFormat);
}

void Dispatcher::glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    OpenglRedirectorBase::glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    if (xoffset > 0 || yoffset > 0)
    {
        Logger::logDebug("Skipping setStorage() due to xoffset != 0 || yoffset != 0");
        return;
    }
    auto finalFormat = hi::trackers::TextureTracker::convertToSizedFormat(format, type);
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, height, level, 0, finalFormat);
}

void Dispatcher::glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
    OpenglRedirectorBase::glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    if (xoffset > 0 || yoffset > 0 || zoffset > 0)
    {
        Logger::logDebug("Skipping setStorage() due to xoffset != 0 || yoffset != 0 || zoffset != 0");
        return;
    }
    auto finalFormat = hi::trackers::TextureTracker::convertToSizedFormat(format, type);
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, height, level, depth, finalFormat);
}

void Dispatcher::glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    OpenglRedirectorBase::glTexStorage1D(target, levels, internalformat, width);
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, 0, levels, 0, internalformat);
}
void Dispatcher::glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glTexStorage2D(target, levels, internalformat, width, height);
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, height, levels, 0, internalformat);
}
void Dispatcher::glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    OpenglRedirectorBase::glTexStorage3D(target, levels, internalformat, width, height, depth);
    m_Context.getTextureTracker().get(getCurrentID(hi::trackers::TextureTracker::getParameterForType(target)))->setStorage(target, width, height, levels, depth, internalformat);
}

void Dispatcher::glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
    OpenglRedirectorBase::glTextureStorage1D(texture, levels, internalformat, width);
    m_Context.getTextureTracker().get(texture)->setStorage(GL_TEXTURE_1D, width, 0, levels, 0, internalformat);
}
void Dispatcher::glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glTextureStorage2D(texture, levels, internalformat, width, height);
    m_Context.getTextureTracker().get(texture)->setStorage(GL_TEXTURE_2D, width, height, levels, 0, internalformat);
}
void Dispatcher::glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    OpenglRedirectorBase::glTextureStorage3D(texture, levels, internalformat, width, height, depth);
    m_Context.getTextureTracker().get(texture)->setStorage(GL_TEXTURE_3D, width, height, levels, depth, internalformat);
}

void Dispatcher::glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    OpenglRedirectorBase::glGenRenderbuffers(n, renderbuffers);

    for (size_t i = 0; i < n; i++)
    {
        m_Context.getRenderbufferTracker().add(renderbuffers[i], std::make_shared<hi::trackers::RenderbufferMetadata>(renderbuffers[i]));
    }
}

void Dispatcher::glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    OpenglRedirectorBase::glDeleteRenderbuffers(n, renderbuffers);
    for (size_t i = 0; i < n; i++)
    {
        m_Context.getRenderbufferTracker().remove(renderbuffers[i]);
    }
}

void Dispatcher::glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glRenderbufferStorage(target, internalformat, width, height);
    // Fix internal format when transfering renderbuffer to texture
    //internalformat = (internalformat == GL_DEPTH_COMPONENT?GL_DEPTH_COMPONENT32:internalformat);
    m_Context.getRenderbufferTracker().get(getCurrentID(GL_RENDERBUFFER_BINDING))->setStorage(target, width, height, 0, 0, internalformat);
}

void Dispatcher::glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glRenderbufferStorage(target, internalformat, width, height);
    // Fix internal format when transfering renderbuffer to texture
    //internalformat = (internalformat == GL_DEPTH_COMPONENT?GL_DEPTH_COMPONENT32:internalformat);
    m_Context.getRenderbufferTracker().get(getCurrentID(GL_RENDERBUFFER_BINDING))->setStorage(target, width, height, 0, 0, internalformat);
}

void Dispatcher::glBindTexture(GLenum target, GLuint texture)
{
    m_Context.getTextureTracker().bind(target, texture);
    auto fakeTextureId = texture;

    if (m_Context.m_IsMultiviewActivated)
    {
        if (m_Context.getTextureTracker().has(texture) && m_Context.getTextureTracker().get(texture)->hasShadowTexture())
        {
            fakeTextureId = m_Context.getTextureTracker().get(texture)->getTextureViewIdOfShadowedTexture();
        }
    }
    OpenglRedirectorBase::glBindTexture(target, fakeTextureId);
}

void Dispatcher::glActiveTexture(GLenum texture)
{
    OpenglRedirectorBase::glActiveTexture(texture);
    m_Context.getTextureTracker().activate(texture - GL_TEXTURE0);
}

GLuint Dispatcher::glCreateShader(GLenum shaderType)
{
    return m_ShaderManager.createShader(m_Context, shaderType);
}

void Dispatcher::glDeleteShader(GLuint shader)
{
    m_ShaderManager.deleteShader(m_Context, shader);
}

void Dispatcher::glShaderSource(GLuint shaderId, GLsizei count, const GLchar* const* string, const GLint* length)
{
    m_ShaderManager.shaderSource(m_Context, shaderId, count, string, length);
}

void Dispatcher::glLinkProgram(GLuint programId)
{
    m_ShaderManager.linkProgram(m_Context, programId);
}

void Dispatcher::glCompileShader(GLuint shader)
{
    m_ShaderManager.compileShader(m_Context, shader);
}

void Dispatcher::glAttachShader(GLuint program, GLuint shader)
{
    m_ShaderManager.attachShader(m_Context, program, shader);
}

GLuint Dispatcher::glCreateProgram(void)
{
    return m_ShaderManager.createProgram(m_Context);
}

void Dispatcher::glDeleteProgram(GLuint program)
{
    return m_ShaderManager.deleteProgram(m_Context, program);
}

void Dispatcher::glUseProgram(GLuint program)
{
    m_ShaderManager.useProgram(m_Context, program);
}

void Dispatcher::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    OpenglRedirectorBase::glUniformMatrix4fv(location, count, transpose, value);

    // get current's program transformation matrix name
    if (!m_Context.getManager().hasBounded())
    {
        Logger::logDebugPerFrame("glUniformMatrix4fv called without bound program!", HI_POS);
        return;
    }
    auto program = m_Context.getManager().getBound();
    if (!program->isInjected())
        return;
    auto metaData = program->m_Metadata.get();

    if (!metaData->hasDetectedTransformation())
        return;

    auto programID = m_Context.getManager().getBoundId();
    // get original MVP matrix location
    auto originalLocation = OpenglRedirectorBase::glGetUniformLocation(programID, metaData->m_TransformationMatrixName.c_str());

    // if the matrix being uploaded isn't detected MVP, then continue
    if (originalLocation != location)
        return;

    // estimate projection matrix from value
    const auto mat = opengl_utils::createMatrixFromRawGL(value);
    auto estimatedParameters = hi::pipeline::estimatePerspectiveProjection(mat);

    auto& ep = estimatedParameters;
    Logger::logDebugPerFrame("estimating parameters from uniform matrix");
    Logger::logDebugPerFrame("parameters: fx(", ep.fx, ") fy(", ep.fy, ") near (", ep.nearPlane, ") far (", ep.farPlane, ") isPerspective (", ep.isPerspective, ")");

    m_DrawManager.setInjectorDecodedProjection(m_Context, programID, estimatedParameters);
}

void Dispatcher::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glViewport(x, y, width, height);
    m_Context.getCurrentViewport().set(x, y, width, height);
    m_Context.getCameras().updateViewports(m_Context.getCurrentViewport());
}

void Dispatcher::glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    OpenglRedirectorBase::glScissor(x, y, width, height);
    m_Context.getCurrentScissorArea().set(x, y, width, height);
}

//-----------------------------------------------------------------------------
// Duplicate API calls
//-----------------------------------------------------------------------------

void Dispatcher::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawArrays(mode, first, count); });
}

void Dispatcher::glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawArraysInstanced(mode, first, count, instancecount); });
}

void Dispatcher::glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawElements(mode, count, type, indices); });
}

void Dispatcher::glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawElementsInstanced(mode, count, type, indices, instancecount); });
}

void Dispatcher::glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawRangeElements(mode, start, end, count, type, indices); });
}

void Dispatcher::glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawElementsBaseVertex(mode, count, type, indices, basevertex); });
}
void Dispatcher::glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex); });
}
void Dispatcher::glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex); });
}

void Dispatcher::glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex)
{
    m_DrawManager.draw(m_Context, [&]() { OpenglRedirectorBase::glMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex); });
}

// ----------------------------------------------------------------------------

void Dispatcher::glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    OpenglRedirectorBase::glGenFramebuffers(n, framebuffers);
    for (size_t i = 0; i < n; i++)
    {
        auto fbo = std::make_shared<hi::trackers::FramebufferMetadata>(framebuffers[i]);
        m_Context.getFBOTracker().add(framebuffers[i], fbo);
    }
}

void Dispatcher::glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    OpenglRedirectorBase::glDeleteFramebuffers(n, framebuffers);
    for (size_t i = 0; i < n; i++)
    {
        m_Context.getFBOTracker().remove(framebuffers[i]);
    }
}

void Dispatcher::glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    m_FramebufferManager.bindFramebuffer(m_Context, target, framebuffer);
}

void Dispatcher::glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture(target, attachment, texture, level);

    assert(m_Context.getTextureTracker().has(texture));
    m_Context.getFBOTracker().getBound()->attach(attachment, m_Context.getTextureTracker().get(texture));
}

void Dispatcher::glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture1D(target, attachment, textarget, texture, level);
    m_Context.getFBOTracker().getBound()->attach(attachment, m_Context.getTextureTracker().get(texture), attachment);
}
void Dispatcher::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    OpenglRedirectorBase::glFramebufferTexture2D(target, attachment, textarget, texture, level);
    m_Context.getFBOTracker().getBound()->attach(attachment, m_Context.getTextureTracker().get(texture), attachment);
}
void Dispatcher::glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    OpenglRedirectorBase::glFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
    m_Context.getFBOTracker().getBound()->attach(attachment, m_Context.getTextureTracker().get(texture), attachment);
}

void Dispatcher::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    OpenglRedirectorBase::glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
    m_Context.getFBOTracker().getBound()->attach(attachment, m_Context.getRenderbufferTracker().get(renderbuffer), attachment);
}
// ----------------------------------------------------------------------------
GLuint Dispatcher::glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
{
    auto result = OpenglRedirectorBase::glGetUniformBlockIndex(program, uniformBlockName);
    if (!m_Context.getManager().has(program))
        return result;
    auto record = m_Context.getManager().get(program);
    if (record->m_UniformBlocks.count(uniformBlockName) == 0)
    {
        hi::trackers::ShaderProgram::UniformBlock block;
        block.location = result;
        record->m_UniformBlocks[std::string(uniformBlockName)] = block;
    }
    return result;
}

void Dispatcher::glDrawBuffers(GLsizei n, const GLenum* bufs)
{
    OpenglRedirectorBase::glDrawBuffers(n, bufs);
    if (m_Context.getFBOTracker().hasBounded())
    {
        auto fbo = m_Context.getFBOTracker().getBound();
        std::vector<GLenum> buffers;
        buffers.assign(bufs, bufs + n);
        fbo->setDrawBuffers(buffers);
    }
}

void Dispatcher::glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    OpenglRedirectorBase::glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    if (!m_Context.getManager().has(program))
        return;

    auto record = m_Context.getManager().get(program);
    record->updateUniformBlock(uniformBlockIndex, uniformBlockBinding);

    // Add binding index's transformation metadata
    if (record->m_Metadata)
    {
        const auto desc = record->m_Metadata.get();
        if (desc->isUBOused() && record->hasUniformBlock(desc->m_InterfaceBlockName))
        {
            auto& block = record->m_UniformBlocks[desc->m_InterfaceBlockName];
            if (OpenglRedirectorBase::glGetUniformBlockIndex(program, desc->m_InterfaceBlockName.c_str()) == uniformBlockIndex)
            {
                auto& index = m_Context.getUniformBlocksTracker().getBindingIndex(block.bindingIndex);

                std::array<const GLchar*, 1> uniformList = { desc->m_TransformationMatrixName.c_str() };
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

void Dispatcher::glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    OpenglRedirectorBase::glBindBufferRange(target, index, buffer, offset, size);
    if (target == GL_UNIFORM_BUFFER)
        m_Context.getUniformBlocksTracker().setUniformBinding(buffer, index);
}
void Dispatcher::glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    OpenglRedirectorBase::glBindBufferBase(target, index, buffer);
    if (target == GL_UNIFORM_BUFFER)
        m_Context.getUniformBlocksTracker().setUniformBinding(buffer, index);
}

void Dispatcher::glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint* buffers)
{
    OpenglRedirectorBase::glBindBuffersBase(target, first, count, buffers);

    if (target == GL_UNIFORM_BUFFER)
    {
        for (size_t i = 0; i < count; i++)
        {
            m_Context.getUniformBlocksTracker().setUniformBinding(buffers[i], first + i);
        }
    }
}

void Dispatcher::glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes)
{
    OpenglRedirectorBase::glBindBuffersRange(target, first, count, buffers, offsets, sizes);

    if (target == GL_UNIFORM_BUFFER)
    {
        for (size_t i = 0; i < count; i++)
        {
            m_Context.getUniformBlocksTracker().setUniformBinding(buffers[i], first + i);
        }
    }
}

void Dispatcher::glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    OpenglRedirectorBase::glBufferData(target, size, data, usage);

    if (target != GL_UNIFORM_BUFFER)
        return;

    GLint bufferID = getCurrentID(GL_UNIFORM_BUFFER_BINDING);
    if (!m_Context.getUniformBlocksTracker().hasBufferBindingIndex(bufferID))
        return;
    auto index = m_Context.getUniformBlocksTracker().getBufferBindingIndex(bufferID);
    auto& metadata = m_Context.getUniformBlocksTracker().getBindingIndex(index);
    if (metadata.transformationOffset == -1)
    {
        // Find a program whose uniform iterface contains transformation matrix
        for (auto& [programID, program] : m_Context.getManager().getMap())
        {
            auto& blocks = program->m_UniformBlocks;

            // Determine fi shader program contains a block, whose binding index is same as
            // currently bound GL_UNIFORM_BUFFER
            auto result = std::find_if(blocks.begin(), blocks.end(), [&](const auto& block) -> bool {
                return (block.second.bindingIndex == index);
            });
            // If index is 0, than any block would do
            if (result == blocks.end() && index != 0)
                continue;
            // Determine if program's VS has transformation in interface block
            if (!program->m_Metadata)
                continue;
            const auto programMetadata = program->m_Metadata.get();
            if (!programMetadata->isUBOused())
                continue;

            std::array<const GLchar*, 1> uniformList = { programMetadata->m_TransformationMatrixName.c_str() };
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
    if (metadata.transformationOffset != -1)
    {
        if (size >= metadata.transformationOffset + sizeof(float) * 16)
        {
            std::memcpy(glm::value_ptr(metadata.transformation), static_cast<const std::byte*>(data) + metadata.transformationOffset, sizeof(float) * 16);
            auto estimatedParameters = hi::pipeline::estimatePerspectiveProjection(metadata.transformation);

            Logger::logDebugPerFrame("estimating parameters from UBO");

            auto& ep = estimatedParameters;
            Logger::logDebugPerFrame("parameters: fx(", ep.fx, ") fy(", ep.fy, ") near (", ep.nearPlane, ") far (", ep.farPlane, ") isPerspective (", ep.isPerspective, ")");

            // TODO: refactor into class method of Binding index structure
            metadata.projection = estimatedParameters;
            metadata.hasTransformation = true;
        }
    }
}
void Dispatcher::glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    OpenglRedirectorBase::glBufferSubData(target, offset, size, data);

    if (target != GL_UNIFORM_BUFFER)
        return;

    GLint bufferID = 0;
    OpenglRedirectorBase::glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &bufferID);
    if (!m_Context.getUniformBlocksTracker().hasBufferBindingIndex(bufferID))
        return;
    auto index = m_Context.getUniformBlocksTracker().getBufferBindingIndex(bufferID);
    auto& metadata = m_Context.getUniformBlocksTracker().getBindingIndex(index);
    if (metadata.transformationOffset != -1 && offset <= metadata.transformationOffset)
    {
        if (offset + size >= metadata.transformationOffset + sizeof(float) * 16)
        {
            std::memcpy(glm::value_ptr(metadata.transformation), static_cast<const std::byte*>(data) + metadata.transformationOffset, sizeof(float) * 16);
            auto estimatedParameters = hi::pipeline::estimatePerspectiveProjection(metadata.transformation);
            Logger::logDebugPerFrame("estimating parameters from UBO");
            auto& ep = estimatedParameters;
            Logger::logDebugPerFrame("parameters: fx(", ep.fx, ") fy(", ep.fy, ") near (", ep.nearPlane, ") near(", ep.farPlane, ")");

            metadata.projection = estimatedParameters;
            metadata.hasTransformation = true;
        }
    }
}

// ----------------------------------------------------------------------------
void Dispatcher::glMatrixMode(GLenum mode)
{
    OpenglRedirectorBase::glMatrixMode(mode);
    m_Context.getLegacyTracker().matrixMode(mode);
    //Logger::log("glMatrixMode ", hi::opengl_utils::getEnumStringRepresentation(mode).c_str());
}
void Dispatcher::glLoadMatrixd(const GLdouble* m)
{
    OpenglRedirectorBase::glLoadMatrixd(m);
    if (m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}
void Dispatcher::glLoadMatrixf(const GLfloat* m)
{
    OpenglRedirectorBase::glLoadMatrixf(m);
    if (m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}

void Dispatcher::glLoadIdentity(void)
{
    OpenglRedirectorBase::glLoadIdentity();
    if (m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        glm::mat4 identity = glm::mat4(1.0);
        m_Context.getLegacyTracker().loadMatrix(std::move(identity));
    }
}

void Dispatcher::glMultMatrixd(const GLdouble* m)
{
    OpenglRedirectorBase::glMultMatrixd(m);
    if (m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
}

void Dispatcher::glMultMatrixf(const GLfloat* m)
{
    OpenglRedirectorBase::glMultMatrixf(m);
    if (m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
}

void Dispatcher::glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val)
{
    OpenglRedirectorBase::glOrtho(left, right, bottom, top, near_val, far_val);
    m_Context.getLegacyTracker().multMatrix(glm::ortho(left, right, bottom, top, near_val, far_val));
}

void Dispatcher::glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val)
{
    OpenglRedirectorBase::glFrustum(left, right, bottom, top, near_val, far_val);
    m_Context.getLegacyTracker().multMatrix(glm::frustum(left, right, bottom, top, near_val, far_val));
}

void Dispatcher::glBegin(GLenum mode)
{
    if (m_Context.m_callList == 0)
    {
        m_Context.m_callList = OpenglRedirectorBase::glGenLists(1);
    }
    OpenglRedirectorBase::glNewList(m_Context.m_callList, GL_COMPILE);
    OpenglRedirectorBase::glBegin(mode);
}

void Dispatcher::glEnd()
{
    OpenglRedirectorBase::glEnd();
    OpenglRedirectorBase::glEndList();

    m_DrawManager.draw(m_Context, [&]() {
        OpenglRedirectorBase::glCallList(m_Context.m_callList);
    });
}

void Dispatcher::glCallList(GLuint list)
{
    m_DrawManager.draw(m_Context, [&]() {
        OpenglRedirectorBase::glCallList(list);
    });
}
void Dispatcher::glCallLists(GLsizei n, GLenum type, const GLvoid* lists)
{
    m_DrawManager.draw(m_Context, [&]() {
        OpenglRedirectorBase::glCallLists(n, type, lists);
    });
}

//-----------------------------------------------------------------------------
// Debugging utils
//-----------------------------------------------------------------------------
int Dispatcher::XNextEvent(Display* display, XEvent* event_return)
{
    //return OpenglRedirectorBase::XNextEvent(display, event_return);
    return m_Context.getX11Sniffer().onXNextEvent(display, event_return);
}

int Dispatcher::XWarpPointer(Display* display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y)
{
    return OpenglRedirectorBase::XWarpPointer(display, src_w, dest_w, src_x, src_y, src_width, src_height, dest_x, dest_y);
}
Window Dispatcher::XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int classInstance, Visual* visual, unsigned long valuemask, XSetWindowAttributes* attributes)
{
    if (width > 1 && height > 1)
    {
        //width = 2560;
        //height = 1600;
        if (attributes)
        {
            attributes->event_mask |= PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
            attributes->do_not_propagate_mask = NoEventMask;
            //attributes->override_redirect = True;
        }
    }
    else
    {
        Logger::logDebug("XCreateWindow called with resolution ", width, "x", height, ", assuming input window");
    }
    return OpenglRedirectorBase::XCreateWindow(display, parent, x, y, width, height, border_width, depth, classInstance, visual, valuemask, attributes);
}

int Dispatcher::XMapWindow(Display* display, Window win)
{
    if (!m_Context.keepWindowInBackgroundFlag)
    {
        return OpenglRedirectorBase::XMapWindow(display, win);
    }
    return 0;
}

void Dispatcher::XSetWMNormalHints(Display* display, Window w, XSizeHints* hints)
{
    if (hints)
    {
        hints->max_width = 2560;
        hints->max_height = 1600;
    }
    OpenglRedirectorBase::XSetWMNormalHints(display, w, hints);
}
