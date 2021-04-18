/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        context.cpp
*
*****************************************************************************/

#include "context.hpp"

#include "trackers/framebuffer_tracker.hpp"
#include "trackers/legacy_tracker.hpp"
#include "trackers/renderbuffer_tracker.hpp"
#include "trackers/shader_tracker.hpp"
#include "trackers/texture_tracker.hpp"
#include "trackers/uniform_block_tracing.hpp"

#include "pipeline/camera_parameters.hpp"
#include "pipeline/output_fbo.hpp"
#include "pipeline/viewport_area.hpp"
#include "pipeline/virtual_cameras.hpp"

#include "diagnostics.hpp"
#include "imgui_adapter.hpp"
#include "ui/settings_widget.hpp"
#include "ui/x11_sniffer.hpp"

namespace hi
{
/**
     * @brief Stores all OpenGL-context related structures
     */
class ContextPimpl
{
public:
    /* ------------------------------------------------------------------------
         * OPTIONS
         * ----------------------------------------------------------------------*/
    /// Is repeater rendering scene into multiple virtual screens
    bool m_IsMultiviewActivated = false;
    /// Determines how virtual views are placed in view-space
    hi::pipeline::CameraParameters m_cameraParameters;
    /// Store's repeating setup
    hi::pipeline::VirtualCameras m_cameras;
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
    hi::trackers::ShaderTracker m_Manager;
    /// Store metadata about create Frame Buffer Objects
    hi::trackers::FramebufferTracker m_FBOTracker;
    /// Keeps track of OpenGL fixed-pipeline calls
    hi::trackers::LegacyTracker m_LegacyTracker;
    /// Keeps track of each GL_TEXTURE_XYZ
    hi::trackers::TextureTracker m_TextureTracker;
    /// Keeps track of GL_RENDERBUFFER objects
    hi::trackers::RenderbufferTracker m_RenderbufferTracker;
    /// Store metadata and bindings for UBO
    hi::trackers::UniformBlockTracing m_UniformBlocksTracker;

    /* ------------------------------------------------------------------------
         *  HELPER STRUCTURES
         * ----------------------------------------------------------------------*/
    /// Caches current viewport/scissor area
    hi::pipeline::ViewportArea currentViewport, currentScissorArea;
    /*
         * Global glCallList for legacy OpenGL primitives
         * - record everything between glBegin()/glEnd() 
         *   and then, duplicate it
         */
    GLint m_callList = 0;

    /// FBO with all raw virtual cameras
    hi::pipeline::OutputFBO m_OutputFBO;
};

Context::Context()
{
    pimpl = std::make_unique<ContextPimpl>();
}

Context::~Context() = default;

void Context::reset()
{
    pimpl = std::make_unique<ContextPimpl>();
}

hi::pipeline::CameraParameters& Context::getCameraParameters()
{
    return pimpl->m_cameraParameters;
}
hi::pipeline::VirtualCameras& Context::getCameras()
{
    return pimpl->m_cameras;
}
Diagnostics& Context::getDiagnostics()
{
    return pimpl->m_diagnostics;
}

ImguiAdapter& Context::getGui()
{
    return pimpl->m_gui;
}

SettingsWidget& Context::getSettingsWidget()
{
    return pimpl->m_settingsWidget;
}

X11Sniffer& Context::getX11Sniffer()
{
    return pimpl->m_x11Sniffer;
}

hi::trackers::ShaderTracker& Context::getManager()
{
    return pimpl->m_Manager;
}

hi::trackers::FramebufferTracker& Context::getFBOTracker()
{
    return pimpl->m_FBOTracker;
}
hi::trackers::LegacyTracker& Context::getLegacyTracker()
{
    return pimpl->m_LegacyTracker;
}
hi::trackers::TextureTracker& Context::getTextureTracker()
{
    return pimpl->m_TextureTracker;
}
hi::trackers::RenderbufferTracker& Context::getRenderbufferTracker()
{
    return pimpl->m_RenderbufferTracker;
}

hi::trackers::UniformBlockTracing& Context::getUniformBlocksTracker()
{
    return pimpl->m_UniformBlocksTracker;
}

hi::pipeline::ViewportArea& Context::getCurrentViewport()
{
    return pimpl->currentViewport;
}

hi::pipeline::ViewportArea& Context::getCurrentScissorArea()
{
    return pimpl->currentScissorArea;
}

hi::pipeline::OutputFBO& Context::getOutputFBO()
{
    return pimpl->m_OutputFBO;
}
}
