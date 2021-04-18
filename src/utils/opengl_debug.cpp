/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_debug.cpp
*
*****************************************************************************/

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glext.h"

#include "logger.hpp"
#include "opengl_debug.hpp"

#include "GL/glu.h"

namespace hi::debug
{
void logOpenglDebugMessage(std::string file, size_t location, GLenum errorCode)
{
    Logger::logError("OpenGL error:", errorCode, "(", reinterpret_cast<const char*>(gluErrorString(errorCode)), ")", " at ", file, ":", location);
}

std::string convertErrorToString(GLenum errorCode)
{
    return reinterpret_cast<const char*>(gluErrorString(errorCode));
}

void logOpenglDebugMessageStr(std::string file, size_t location, std::string error)
{
    Logger::logError("OpenGL error:", error, " at ", file, ":", location);
}

void logTrace(const std::string msg)
{
    glGetUniformLocation(0, ("trace: " + msg).c_str());
    CLEAR_GL_ERROR();
}
}
