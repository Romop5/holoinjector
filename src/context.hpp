#ifndef VE_CONTEXT_HPP
#define VE_CONTEXT_HPP

#include "trackers/shader_manager.hpp"
#include "trackers/uniform_block_tracing.hpp"
#include "trackers/framebuffer_tracker.hpp"
#include "trackers/legacy_tracker.hpp"
#include "trackers/texture_tracker.hpp"
#include "trackers/renderbuffer_tracker.hpp"

#include "pipeline/viewport_area.hpp"
#include "pipeline/camera_parameters.hpp"
#include "pipeline/virtual_cameras.hpp"
#include "pipeline/output_fbo.hpp"

#include "diagnostics.hpp"
#include "imgui_adapter.hpp"
#include "ui/x11_sniffer.hpp"
#include "ui/settings_widget.hpp"

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
    ve::pipeline::CameraParameters m_cameraParameters;
    /// Store's repeating setup
    ve::pipeline::VirtualCameras m_cameras;
    /// Provides interface for system testing
    Diagnostics m_diagnostics;

    /// Dear ImGUI Adapter
    ImguiAdapter m_gui;

    /// UI controller for settings
    SettingsWidget m_settingsWidget;

    X11Sniffer m_x11Sniffer;
    /* ------------------------------------------------------------------------
     *  TRACKERS
     * ----------------------------------------------------------------------*/
    /// Store metadata about application's shaders and programs
    ve::trackers::ShaderManager m_Manager;
    /// Store metadata about create Frame Buffer Objects
    ve::trackers::FramebufferTracker m_FBOTracker;
    /// Keeps track of OpenGL fixed-pipeline calls
    ve::trackers::LegacyTracker m_LegacyTracker;
    /// Keeps track of each GL_TEXTURE_XYZ
    ve::trackers::TextureTracker m_TextureTracker;
    /// Keeps track of GL_RENDERBUFFER objects
    ve::trackers::RenderbufferTracker m_RenderbufferTracker;
    /// Store metadata and bindings for UBO
    ve::trackers::UniformBlockTracing m_UniformBlocksTracker;

    /* ------------------------------------------------------------------------
     *  HELPER STRUCTURES
     * ----------------------------------------------------------------------*/
    /// Caches current viewport/scissor area
    ve::pipeline::ViewportArea currentViewport, currentScissorArea;
    /*
     * Global glCallList for legacy OpenGL primitives
     * - record everything between glBegin()/glEnd() 
     *   and then, duplicate it
     */
    GLint m_callList = 0;

    /// FBO with all raw virtual cameras
    ve::pipeline::OutputFBO m_OutputFBO;
};
} // namespace ve
#endif

