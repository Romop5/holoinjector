#include <cassert>

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "utils/opengl_objects.hpp"

ve::utils::glObject::~glObject()
{
    if(m_ID == 0)
        return;
    if(glIsProgram(m_ID))
        glDeleteProgram(m_ID);
    else if(glIsShader(m_ID))
        glDeleteShader(m_ID);
    else if(glIsVertexArray(m_ID))
    {
        GLuint id = m_ID;
        glDeleteVertexArrays(1,&id);
    }
    else if(glIsBuffer(m_ID))
    {
        GLuint id = m_ID;
        glDeleteBuffers(1, &id);
    }
    else if(glIsFramebuffer(m_ID))
    {
        GLuint id = m_ID;
        glDeleteFramebuffers(1, &id);
    }
    m_ID = 0;
}

bool ve::utils::glShader::create(const std::string& src, GLenum type)
{
    auto shaderId = glCreateShader(type);
    const char* code = src.c_str();
    glShaderSource(shaderId, 1, &code,nullptr);
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
        Logger::log("[Repeater] Failed to compile", log);
        Logger::log("[Repeater] code", src.c_str());
        std::fflush(stdout);
    }
    setID(shaderId);
    assert(compileStatus == GL_TRUE);
    return shaderId;
}

bool ve::utils::glProgram::link(const std::vector<size_t> shaders)
{
    auto program = glCreateProgram();

    for(const auto& shader: shaders)
        glAttachShader(program, shader);

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    assert(linkStatus == GL_TRUE);
    setID(program);
    return true;
}

void ve::utils::glFullscreenVAO::create()
{
    struct VertexData
    {
        float position[3];
        float uv[2];
    };

    VertexData vertices[] =
    {
        {-1.0, -1.0, 0.0, 0.0,0.0},
        {1.0, -1.0, 0.0, 1.0,0.0},
        {-1.0, 1.0, 0.0, 0.0,1.0},
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

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    // Enable coords
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(float),0);
    glEnableVertexAttribArray(0);
    // Enable UV
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(float),reinterpret_cast<void*>(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, oldVBO);
    glBindVertexArray(oldVao);

    m_VBO = std::move(glObject(vertexBuffer));
    setID(vao);
}

void ve::utils::glFullscreenVAO::draw()
{
    GLint oldVao;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVao);
    glBindVertexArray(getID());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(oldVao);
}

