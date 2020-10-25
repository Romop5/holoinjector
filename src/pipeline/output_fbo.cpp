#include <cassert>
#include <string>

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glext.h"

#include "pipeline/output_fbo.hpp"

using namespace ve;

void OutputFBO::initialize()
{
    glGenFramebuffers(1, &m_FBOId); 
    assert(glGetError() == GL_NO_ERROR);
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBOId);
    assert(glGetError() == GL_NO_ERROR);

    glGenTextures(1, &m_LayeredColorBuffer);
    assert(glGetError() == GL_NO_ERROR);
    glGenTextures(1, &m_LayeredDepthStencilBuffer);
    assert(glGetError() == GL_NO_ERROR);


    glBindTexture(GL_TEXTURE_2D_ARRAY, m_LayeredColorBuffer);
    assert(glGetError() == GL_NO_ERROR);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, fbo_pixels_width,fbo_pixels_height, 9);
    assert(glGetError() == GL_NO_ERROR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_ARRAY, m_LayeredColorBuffer, 0);
    auto error = glGetError();
    if(error != GL_NO_ERROR)
    {
        printf("[Repeater] error: %d\n", error);
    }
    assert(error == GL_NO_ERROR);

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_LayeredDepthStencilBuffer);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH24_STENCIL8, fbo_pixels_width,fbo_pixels_height, 9);
    assert(glGetError() == GL_NO_ERROR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_ARRAY, m_LayeredDepthStencilBuffer, 0);
    assert(glGetError() == GL_NO_ERROR);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("[Repeater] Failed to create FBO for layered rendering: Status: glEnum %d\n", status);
    }
    assert(status == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);


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
        uniform int enhancer_max_layers = 1;
        uniform sampler2DArray enhancer_layeredScreen;
        in vec2 uv;
        out vec4 color;
        void main()
        {
            vec2 newUv = mod(3.0*uv, 1.0);
            ivec2 indices = ivec2(int(newUv.x*3.0),int(newUv.y*3.0));
            int layer = indices.y*3+indices.x;
            color = 0.5*texture(enhancer_layeredScreen, vec3(newUv, layer)) + 0.5*vec4(newUv, 0.0,1.0);
            color.w = 1.0;
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

    GLint oldVao;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);
    GLint oldVBO;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldVBO);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    // Enable coords
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(float),0);
    glEnableVertexAttribArray(0);
    // Enable UV
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(float),reinterpret_cast<void*>(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, oldVBO);
    glBindVertexArray(oldVao);
}

void OutputFBO::renderToBackbuffer()
{
    GLint oldProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

    GLint oldVao;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);

    glUseProgram(m_ViewerProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(m_VAO);   
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_LayeredColorBuffer);
    glActiveTexture(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBOId);

    glUseProgram(oldProgram);
    glBindVertexArray(oldVao);
}

GLuint OutputFBO::getFBOId()
{
    return m_FBOId;
}
