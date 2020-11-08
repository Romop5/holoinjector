#ifndef REPEATER_OUTPUT_FBO_HPP
#define REPEATER_OUTPUT_FBO_HPP

#include "GL/gl.h"

namespace ve
{
    class OutputFBOParameters
    {
        public:
        OutputFBOParameters() = default;
	size_t getTextureWidth() const;
	size_t getTextureHeight() const;
        size_t getLayers() const;
        size_t getGridSizeX() const;

        protected:
        size_t gridSize = 3;
        size_t pixels_width = 256;
        size_t pixels_height = 256;
    };
    struct OutputFBO
    {
        public:
        OutputFBO() = default;
        ~OutputFBO();
        /// Creates OpenGL objects
        void initialize(OutputFBOParameters params = OutputFBOParameters());
        void deinitialize();
        /// Blits all cameras to back buffer (as a grid)
        void renderToBackbuffer();
        /// Clear buffers on new frame
        void clearBuffers();

        GLuint getFBOId();

        const OutputFBOParameters& getParams();
        private:
        GLuint  m_FBOId = 0;
        GLuint  m_LayeredColorBuffer = 0;
        GLuint  m_LayeredDepthStencilBuffer = 0;

        /// Shader program for displaying layared color buffers
        GLuint  m_ViewerProgram = 0;

        // Full screen quad
        GLuint  m_VAO = 0;

        // Parameters
        OutputFBOParameters m_Params;
    };

}; //namespace ve
#endif
