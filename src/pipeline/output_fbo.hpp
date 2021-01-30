#ifndef REPEATER_OUTPUT_FBO_HPP
#define REPEATER_OUTPUT_FBO_HPP

#include <memory>
#include <vector>
#include "GL/gl.h"
#include "utils/opengl_raii.hpp"
#include "paralax/mapping.hpp"

namespace ve
{
namespace utils
{
    /// Fwd declaration
    class glFullscreenVAO;
}
namespace pipeline
{
    /// Fwd declaration
    class CameraParameters;

    class OutputFBOParameters
    {
        public:
        OutputFBOParameters() = default;
	size_t getTextureWidth() const;
	size_t getTextureHeight() const;
        size_t getLayers() const;
        size_t getGridSizeX() const;
        size_t getGridSizeY() const;

        size_t gridXSize = 5;
        size_t gridYSize = 9;
        size_t pixels_width = 256;
        size_t pixels_height = 256;
    };
    /**
     * @brief Wraps back-buffer with a layered FBO
     *
     * This class shadows a back-buffer. Instead rendering into the back-buffer,
     * hooked draw calls are redirected to layers of OutputFBO. In the end of frame,
     * layers are drawed to back-buffer using grid shader (nt. debug) or quit shader (a native
     * display).
     */
    struct OutputFBO
    {
        public:
        OutputFBO() = default;
        ~OutputFBO();
        /// Creates OpenGL objects
        void initialize(OutputFBOParameters params = OutputFBOParameters());
        /// Clean up
        void deinitialize();
        /// Get reference to stored parameters
        const OutputFBOParameters& getParams();
        /// Return FBO's ID
        GLuint getFBOId();

        /// Blits all cameras to back buffer (as a grid)
        void renderToBackbuffer(const CameraParameters& params);
        /// Clear buffers on new frame
        void clearBuffers();

        /// Sets the flag, determining if any pixel of final image is contained, to on
        void setContainsImageFlag();
        /// Has any valid image in buffer
        bool hasImage() const;

        /// Create proxy FBO from texture views to a single layer of shadow FBO
        GLuint createProxyFBO(size_t layer);

        /// Toggle grid vs native format view
        void toggleGridView();

        /// Toggle
        void toggleSingleViewGridView();

        /// Set id of only-viewed quilt view
        void setOnlyQuiltImageID(size_t id);
        //---------------------------------------------------------------------
        private:
        std::vector<ve::utils::FBORAII> m_proxyFBO;
        void clearImageFlag();

        /// Render layers in grid layout
        void renderGridLayout();
        /// Render paralax
        void renderParalax(const CameraParameters& params);
        bool    m_ContainsImageFlag = false;
        GLuint  m_FBOId = 0;
        GLuint  m_LayeredColorBuffer = 0;
        GLuint  m_LayeredDepthStencilBuffer = 0;

        /// Shader program for displaying layared color buffers
        GLuint  m_ViewerProgram = 0;

        // Full screen quad
        std::shared_ptr<ve::utils::glFullscreenVAO> m_VAO;

        // Parameters
        OutputFBOParameters m_Params;

        paralax::Mapping m_Pm;

        bool shouldDisplayGrid = false;
        bool shouldDisplayOnlySingleQuiltImage = false;
        size_t m_OnlyQuiltImageID = 0;
    };

}; //namespace pipeline
}; //namespace ve
#endif
