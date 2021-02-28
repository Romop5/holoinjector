#include "opengl_debug.hpp"
#include "logger.hpp"

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glu.h"


namespace ve::debug
{
    void logOpenglDebugMessage(std::string file,size_t location, GLenum errorCode)
    {
        Logger::logError("[Repeater] OpenGL error:", errorCode,"(",reinterpret_cast<const char*>(gluErrorString(errorCode)),")", " at ",__FILE__,":",__LINE__);\
    }
}
