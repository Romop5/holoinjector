/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        context.hpp
*
*****************************************************************************/

#ifndef HI_CONTEXT_HPP
#define HI_CONTEXT_HPP
#include <memory>
namespace hi
{
namespace pipeline
{
    class CameraParameters;
    class VirtualCameras;
    class ViewportArea;
    class OutputFBO;
    class ShaderProfile;
}

class Diagnostics;
class ImguiAdapter;
class SettingsWidget;
class X11Sniffer;

namespace trackers
{
    class ShaderTracker;
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
    void reset();
    /* ------------------------------------------------------------------------
     * OPTIONS
     * ----------------------------------------------------------------------*/
    /// Is repeater rendering scene into multiple virtual screens
    bool m_IsMultiviewActivated = false;
    /// Determines how virtual views are placed in view-space
    hi::pipeline::CameraParameters& getCameraParameters();
    /// Store's repeating setup
    hi::pipeline::VirtualCameras& getCameras();
    /// Provides interface for system testing
    Diagnostics& getDiagnostics();
    /// User-defined shader profiles
    hi::pipeline::ShaderProfile& getProfiles();

    /* ------------------------------------------------------------------------
     *  UI
     * ----------------------------------------------------------------------*/

    /// Dear ImGUI Adapter
    ImguiAdapter& getGui();

    /// UI controller for settings
    SettingsWidget& getSettingsWidget();

    X11Sniffer& getX11Sniffer();
    /* ------------------------------------------------------------------------
     *  TRACKERS
     * ----------------------------------------------------------------------*/
    /// Store metadata about application's shaders and programs
    hi::trackers::ShaderTracker& getManager();
    /// Store metadata about create Frame Buffer Objects
    hi::trackers::FramebufferTracker& getFBOTracker();
    /// Keeps track of OpenGL fixed-pipeline calls
    hi::trackers::LegacyTracker& getLegacyTracker();
    /// Keeps track of each GL_TEXTURE_XYZ
    hi::trackers::TextureTracker& getTextureTracker();
    /// Keeps track of GL_RENDERBUFFER objects
    hi::trackers::RenderbufferTracker& getRenderbufferTracker();
    /// Store metadata and bindings for UBO
    hi::trackers::UniformBlockTracing& getUniformBlocksTracker();

    /* ------------------------------------------------------------------------
     *  HELPER STRUCTURES
     * ----------------------------------------------------------------------*/
    /// Caches current viewport/scissor area
    hi::pipeline::ViewportArea& getCurrentViewport();
    hi::pipeline::ViewportArea& getCurrentScissorArea();
    /*
     * Global glCallList for legacy OpenGL primitives
     * - record everything between glBegin()/glEnd() 
     *   and then, duplicate it
     */
    uint32_t m_callList = 0;

    /// FBO with all raw virtual cameras
    hi::pipeline::OutputFBO& getOutputFBO();

    /// Determines if newly create GL window should be put to background
    bool keepWindowInBackgroundFlag = false;

    /// Use Vertex Shader instead of Geometry Shader
    bool dontInsertGeometryShader = false;

private:
    std::unique_ptr<ContextPimpl> pimpl;
};
} // namespace hi
#endif
