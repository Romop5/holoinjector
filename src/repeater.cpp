#include "repeater.hpp"
#include <iostream>

using namespace ve;

void Repeater::registerCallbacks() 
{
    registerOpenGLSymbols();
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

//-----------------------------------------------------------------------------
// Duplicate API calls
//-----------------------------------------------------------------------------

void Repeater::glDrawArrays(GLenum mode,GLint first,GLsizei count) 
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawArrays(mode,first,count);
    });
}

void Repeater::glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices) 
{
    Repeater::duplicateCode([&]() {
        OpenglRedirectorBase::glDrawElements(mode,count,type,indices);
    });

}

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------
void Repeater::duplicateCode(const std::function<void(void)>& code)
{
    GLint viewport[4];
    OpenglRedirectorBase::glGetIntegerv(GL_VIEWPORT, viewport);
    // Set bottom (y is reverted in OpenGL) 
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1], viewport[2],viewport[3]/2);
    code();
    // Set top
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1]+viewport[3]/2, viewport[2],viewport[3]/2);
    code();
    // restore viewport
    OpenglRedirectorBase::glViewport(viewport[0],viewport[1], viewport[2],viewport[3]);
}

