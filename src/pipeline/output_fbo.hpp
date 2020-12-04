#ifndef REPEATER_OUTPUT_FBO_HPP
#define REPEATER_OUTPUT_FBO_HPP

#include <memory>
#include <vector>
#include "GL/gl.h"
#include "utils/opengl_raii.hpp"

namespace ve
{
namespace utils
{
    class glFullscreenVAO;
}
namespace pipeline
{
    

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
        size_t pixels_width = 512;
        size_t pixels_height = 256;
    };
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
        void renderToBackbuffer();
        /// Clear buffers on new frame
        void clearBuffers();

        /// Sets the flag, determining if any pixel of final image is contained, to on
        void setContainsImageFlag();
        /// Has any valid image in buffer
        bool hasImage() const;

        /// Create proxy FBO from texture views to a single layer of shadow FBO
        GLuint createProxyFBO(size_t layer);
        //---------------------------------------------------------------------
        private:
        std::vector<ve::utils::FBORAII> m_proxyFBO;
        void clearImageFlag();
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
    };

}; //namespace pipeline
}; //namespace ve
#endif
