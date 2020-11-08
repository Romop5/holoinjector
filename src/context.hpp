#ifndef VE_CONTEXT_HPP
#define VE_CONTEXT_HPP

#include "trackers/shader_manager.hpp"
#include "trackers/uniform_block_tracing.hpp"
#include "trackers/framebuffer_tracker.hpp"
#include "trackers/legacy_tracker.hpp"
#include "trackers/texture_tracker.hpp"
#include "trackers/renderbuffer_tracker.hpp"

#include "pipeline/viewport_area.hpp"
#include "pipeline/virtual_cameras.hpp"
#include "pipeline/output_fbo.hpp"

#include "diagnostics.hpp"

namespace ve
{
/**
 * @brief Stores all OpenGL-context related structures
 */
struct Context
{
    /* ------------------------------------------------------------------------
     * OPTIONS
     * ----------------------------------------------------------------------*/
    /// Is repeater rendering scene into multiple virtual screens
    bool m_IsMultiviewActivated = false;
    /// Determines how virtual views are placed in view-space
    CameraParameters m_cameraParameters;
    /// Store's repeating setup
    VirtualCameras m_cameras;
    /// Provides interface for system testing
    Diagnostics m_diagnostics;
 
    /* ------------------------------------------------------------------------
     *  TRACKERS
     * ----------------------------------------------------------------------*/
    /// Store metadata about application's shaders and programs
    ShaderManager m_Manager;
    /// Store metadata about create Frame Buffer Objects
    FramebufferTracker m_FBOTracker;
    /// Keeps track of OpenGL fixed-pipeline calls
    LegacyTracker m_LegacyTracker;
    /// Keeps track of each GL_TEXTURE_XYZ
    TextureTracker m_TextureTracker;
    /// Keeps track of GL_RENDERBUFFER objects
    RenderbufferTracker m_RenderbufferTracker;
    /// Store metadata and bindings for UBO
    UniformBlockTracing m_UniformBlocksTracker;

    /* ------------------------------------------------------------------------
     *  HELPER STRUCTURES
     * ----------------------------------------------------------------------*/
    /// Caches current viewport/scissor area
    ViewportArea currentViewport, currentScissorArea;
    /* 
     * Global glCallList for legacy OpenGL primitives
     * - record everything between glBegin()/glEnd() 
     *   and then, duplicate it
     */
    GLint m_callList = 0;

    /// FBO with all raw virtual cameras
    OutputFBO m_OutputFBO;
};
} // namespace ve
#endif

