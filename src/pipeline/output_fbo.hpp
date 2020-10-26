#ifndef REPEATER_OUTPUT_FBO_HPP
#define REPEATER_OUTPUT_FBO_HPP

#include "GL/gl.h"

namespace ve
{
    struct OutputFBOParameters
    {
        OutputFBOParameters() = default;
        size_t gridSize = 3;
        size_t pixels_width = 256;
        size_t pixels_height = 256;
    };
    struct OutputFBO 
    {
        public:
        /// Creates OpenGL objects
        void initialize(OutputFBOParameters params = OutputFBOParameters());
        /// Blits all cameras to back buffer (as a grid)
        void renderToBackbuffer();

	size_t getTextureWidth() const;
	size_t getTextureHeight() const;

        GLuint getFBOId();
        private:
        GLuint  m_FBOId;
        GLuint  m_LayeredColorBuffer;
        GLuint  m_LayeredDepthStencilBuffer;

        /// Shader program for displaying layared color buffers
        GLuint  m_ViewerProgram;

        // Full screen quad
        GLuint  m_VAO;

        // Parameters
        OutputFBOParameters m_Params;
    };

}; //namespace ve
#endif
