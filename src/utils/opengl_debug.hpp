#ifndef OPENGL_DEBUG_HPP
#define OPENGL_DEBUG_HPP

#include <string>
#include "GL/gl.h"

#define CLEAR_GL_ERROR()\
    glGetError();

#define ASSERT_GL_ERROR()\
{\
    auto error = glGetError();\
    if(error != GL_NO_ERROR)\
    {\
        ve::debug::logOpenglDebugMessage(__FILE__,__LINE__, error);\
        assert(false);\
    }\
}

namespace ve
{
namespace debug
{
    void logOpenglDebugMessage(std::string file,size_t location, GLenum errorCode);
}
}

#endif
