#include "context.hpp"

#include "trackers/shader_tracker.hpp"
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
    class ContextPimpl
    {
        public:
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
        ve::trackers::ShaderTracker m_Manager;
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

    Context::Context()
    {
        pimpl = std::make_unique<ContextPimpl>();
    }

    Context::~Context() = default;


    ve::pipeline::CameraParameters& Context::getCameraParameters()
    {
        return pimpl->m_cameraParameters;
    }
    ve::pipeline::VirtualCameras& Context::getCameras()
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

    ve::trackers::ShaderTracker& Context::getManager()
    {
        return pimpl->m_Manager;
    }

    ve::trackers::FramebufferTracker& Context::getFBOTracker()
    {
        return pimpl->m_FBOTracker;
    }
    ve::trackers::LegacyTracker& Context::getLegacyTracker()
    {
        return pimpl->m_LegacyTracker;
    }
    ve::trackers::TextureTracker& Context::getTextureTracker()
    {
        return pimpl->m_TextureTracker;
    }
    ve::trackers::RenderbufferTracker& Context::getRenderbufferTracker()
    {
        return pimpl->m_RenderbufferTracker;
    }

    ve::trackers::UniformBlockTracing& Context::getUniformBlocksTracker()
    {
        return pimpl->m_UniformBlocksTracker;
    }

    ve::pipeline::ViewportArea& Context::getCurrentViewport()
    {
        return pimpl->currentViewport;
    }

    ve::pipeline::ViewportArea& Context::getCurrentScissorArea()
    {
        return pimpl->currentScissorArea;
    }

    ve::pipeline::OutputFBO& Context::getOutputFBO()
    {
        return pimpl->m_OutputFBO;
    }
}
