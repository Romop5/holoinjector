#ifndef VE_CONTEXT_HPP
#define VE_CONTEXT_HPP
#include <memory>
namespace ve
{
    namespace pipeline
    {
        class CameraParameters; 
        class VirtualCameras; 
        class ViewportArea; 
        class OutputFBO;
    }

    class Diagnostics; 
    class ImguiAdapter; 
    class SettingsWidget;
    class X11Sniffer;

    namespace trackers
    {
        class ShaderManager; 
        class FramebufferTracker; 
        class LegacyTracker; 
        class TextureTracker; 
        class RenderbufferTracker; 
        class UniformBlockTracing;
    }

    class ContextPimpl;
/**
 * @brief Stores all OpenGL-context related structures
 */
class Context
{
    public:
    explicit Context();
    ~Context();
    /* ------------------------------------------------------------------------
     * OPTIONS
     * ----------------------------------------------------------------------*/
    /// Is repeater rendering scene into multiple virtual screens
    bool m_IsMultiviewActivated = false;
    /// Determines how virtual views are placed in view-space
    ve::pipeline::CameraParameters& getCameraParameters();
    /// Store's repeating setup
    ve::pipeline::VirtualCameras& getCameras();
    /// Provides interface for system testing
    Diagnostics& getDiagnostics();

    /// Dear ImGUI Adapter
    ImguiAdapter& getGui();

    /// UI controller for settings
    SettingsWidget& getSettingsWidget();

    X11Sniffer& getX11Sniffer();
    /* ------------------------------------------------------------------------
     *  TRACKERS
     * ----------------------------------------------------------------------*/
    /// Store metadata about application's shaders and programs
    ve::trackers::ShaderManager& getManager();
    /// Store metadata about create Frame Buffer Objects
    ve::trackers::FramebufferTracker& getFBOTracker();
    /// Keeps track of OpenGL fixed-pipeline calls
    ve::trackers::LegacyTracker& getLegacyTracker();
    /// Keeps track of each GL_TEXTURE_XYZ
    ve::trackers::TextureTracker& getTextureTracker();
    /// Keeps track of GL_RENDERBUFFER objects
    ve::trackers::RenderbufferTracker& getRenderbufferTracker();
    /// Store metadata and bindings for UBO
    ve::trackers::UniformBlockTracing& getUniformBlocksTracker();

    /* ------------------------------------------------------------------------
     *  HELPER STRUCTURES
     * ----------------------------------------------------------------------*/
    /// Caches current viewport/scissor area
    ve::pipeline::ViewportArea& getCurrentViewport();
    ve::pipeline::ViewportArea& getCurrentScissorArea();
    /*
     * Global glCallList for legacy OpenGL primitives
     * - record everything between glBegin()/glEnd() 
     *   and then, duplicate it
     */
    uint32_t m_callList = 0;

    /// FBO with all raw virtual cameras
    ve::pipeline::OutputFBO& getOutputFBO();
    private:
    std::unique_ptr<ContextPimpl> pimpl;
};
} // namespace ve
#endif

