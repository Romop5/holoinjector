#ifndef REPEATER_OUTPUT_FBO_HPP
#define REPEATER_OUTPUT_FBO_HPP

#include "GL/gl.h"

namespace ve
{
    struct OutputFBOParameters
    {
        OutputFBOParameters() = default;
        size_t gridSize = 10;
    };
    struct OutputFBO 
    {
        
        static constexpr size_t fbo_pixels_width = 128;
        static constexpr size_t fbo_pixels_height = 128;

        public:
        /// Creates OpenGL objects
        void initialize(OutputFBOParameters params = OutputFBOParameters());
        /// Blits all cameras to back buffer (as a grid)
        void renderToBackbuffer();

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
