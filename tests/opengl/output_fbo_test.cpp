#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glext.h"

#include <iostream>
#include <cassert>
#include "opengl_test_context.hpp"

#include "pipeline/output_fbo.hpp"
#include "pipeline/camera_parameters.hpp"
#include "utils/opengl_objects.hpp"


struct Triangle
{
    void initialize()
    {
        /*
         * Create shader program for displaying layered color buffer
         */

        auto VS = std::string(R"(
            #version 440 core
            layout (location = 0) in vec3 position;

            out vec2 uv;
            void main()
            {
                gl_Position = vec4(position,1.0);
                uv = (position.xy+vec2(1.0))/2.0f;
            }
        )");

        auto FS = std::string(R"(
            #version 440 core
            in vec2 uv;
            out vec4 color;
            void main()
            {
                color = vec4(sin(uv.x*20.0),uv.y, 0.0,1.0);
            }
        )");

        auto compileShader = [&](GLenum type,const std::string& sourceCode)->GLuint
        {
            auto shaderId = glCreateShader(type);
            const GLchar* const sources[1] = {sourceCode.c_str(),};
            glShaderSource(shaderId, 1, sources,nullptr);
            glCompileShader(shaderId);
            GLint compileStatus;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
            if(compileStatus != GL_TRUE)
            {
                GLint logSize = 0;
                glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
                
                GLsizei realLogLength = 0;
                GLchar log[5120] = {0,};
                glGetShaderInfoLog(shaderId, logSize, &realLogLength, log);
                std::printf("[Repeater] Failed to compile %s\n", log);
                std::printf("[Repeater] code %s\n", sourceCode.c_str());
                std::fflush(stdout);
            }
            assert(compileStatus == GL_TRUE);
            return shaderId;
        };

        auto fsId = compileShader(GL_FRAGMENT_SHADER, FS);
        auto vsId = compileShader(GL_VERTEX_SHADER, VS);

        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

        auto program = glCreateProgram();
        //glUseProgram(program);
        glAttachShader(program, fsId);
        glAttachShader(program, vsId);

        glLinkProgram(program);
        GLint linkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);


        //glUseProgram(oldProgram);

        assert(linkStatus == GL_TRUE);
        m_ViewerProgram = program;

        /*
         * Create VAO for full screen quad
         */

        struct VertexData
        {
            float position[3];
            float uv[2];
        };
        VertexData vertices[] = 
        {
            {-1.0, -1.0, 0.0, 0.0,0.0},
            {1.0, -1.0, 0.0, 1.0,0.0},
            {-1.0, 1.0, 0.0, -1.0,0.0},
            {1.0, 1.0, 0.0, 1.0,1.0},
        };

        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*5*4, vertices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        // Enable coords
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(float),0);
        glEnableVertexAttribArray(0);
        // Enable UV
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(float),reinterpret_cast<void*>(2*sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void draw()
    {
        glUseProgram(m_ViewerProgram);
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
        glUseProgram(0);
    }
    private:
        /// Shader program for displaying layared color buffers
        GLuint  m_ViewerProgram;

        // Full screen quad
        GLuint  m_VAO;
};

int main(int argc, char** argv) {
    OpenGLTestContext windowSystem;
    windowSystem.initialize();

    hi::pipeline::OutputFBO fbo;
    fbo.initialize();

    Triangle triangle;
    triangle.initialize();

    // Set GL Sample stuff
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    hi::pipeline::CameraParameters params;

        // Enter message loop
    while (true) {
        windowSystem.handleEvents();
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo.getFBOId());
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        triangle.draw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        fbo.renderToBackbuffer(params);

        /*glBegin(GL_TRIANGLES);
                glColor3f(  1.0f,  0.0f, 0.0f);
                glVertex3f( 0.0f, -1.0f, 0.0f);
                glColor3f(  0.0f,  1.0f, 0.0f);
                glVertex3f(-1.0f,  1.0f, 0.0f);
                glColor3f(  0.0f,  0.0f, 1.0f);
                glVertex3f( 1.0f,  1.0f, 0.0f);
        glEnd();
        */

        // Present frame
        glXSwapBuffers(windowSystem.getDisplay(), windowSystem.getWindow());
    }
    
    return 0;
}

