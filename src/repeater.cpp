#include "repeater.hpp"
#include <iostream>

using namespace ve;

void Repeater::registerCallbacks(SymbolRedirection& redirector) 
{
    registerOpenGLSymbols({"glClear"}, redirector);
}

void Repeater::glClear(GLbitfield mask)
{
    printf("[Repeater] Redirecting glClear\n");
    OpenglRedirectorBase::glClear(mask);
} 


void Repeater::glShaderSource (GLuint shader, GLsizei count, const GLchar* const*string, const GLint* length)
{
    printf("[Repeater] glShaderSource: \n");
    for(size_t cnt = 0; cnt < count; cnt++)
    {
        puts("Shader: \n");
        puts(string[cnt]);
        puts("\n");
    }
    OpenglRedirectorBase::glShaderSource(shader,count,string,length);
}

