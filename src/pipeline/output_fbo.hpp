#ifndef REPEATER_OUTPUT_FBO_HPP
#define REPEATER_OUTPUT_FBO_HPP

#include "GL/gl.h"

namespace ve
{
    struct OutputFBO 
    {
        static constexpr size_t fbo_pixels_width = 512;
        static constexpr size_t fbo_pixels_height = 512;

        public:
        /// Creates OpenGL objects
        void initialize();
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
    };

}; //namespace ve
#endif
