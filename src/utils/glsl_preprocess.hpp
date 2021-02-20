#ifndef VE_GLSL_PREPROCESS_HPP
#define VE_GLSL_PREPROCESS_HPP

#include <GL/gl.h>
#include <string>

namespace ve 
{
namespace glsl_preprocess
{
    std::string wrapGLSLMacros(std::string code);
    std::string unwrapGLSLMacros(std::string code);
    std::string preprocessGLSLCode(std::string code);
    std::string joinGLSLshaders(GLsizei count, const GLchar* const*string, const GLint* length);

    std::string removeComments(std::string code);
} //namespace glsl_preprocess
} //namespace ve

#endif
