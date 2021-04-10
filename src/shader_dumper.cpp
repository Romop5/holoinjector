/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        shader_dumper.cpp
*
*****************************************************************************/

#include "shader_dumper.hpp"

using namespace ve;

namespace helper
{
void dumpShaderSources(const GLchar* const* strings, GLsizei count)
{
    for (size_t i = 0; i < count; i++)
    {
        Logger::log("Dumping shader source i: {}\n", i);
        puts(strings[i]);
        Logger::log("---\n");
    }
}
} // namespace helper

void ShaderDumper::registerCallbacks()
{
    registerOpenGLSymbols();
}

GLuint ShaderDumper::glCreateShader(GLenum shaderType)
{
    auto id = OpenglRedirectorBase::glCreateShader(shaderType);
    return id;
}

void ShaderDumper::glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    Logger::log("[ShaderDumper] glShaderSource: \n");
    helper::dumpShaderSources(string, count);
    OpenglRedirectorBase::glShaderSource(shader, count, string, length);
}
void ShaderDumper::glShaderSourceARB(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    Logger::log("[ShaderDumper] glShaderSource: \n");
    helper::dumpShaderSources(string, count);
    OpenglRedirectorBase::glShaderSource(shader, count, string, length);
}
