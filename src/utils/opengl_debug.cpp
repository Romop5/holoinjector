#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glext.h"

#include "opengl_debug.hpp"
#include "logger.hpp"

#include "GL/glu.h"


namespace ve::debug
{
    void logOpenglDebugMessage(std::string file,size_t location, GLenum errorCode)
    {
        Logger::logError("[Repeater] OpenGL error:", errorCode,"(",reinterpret_cast<const char*>(gluErrorString(errorCode)),")", " at ",file,":",location);\
    }

    std::string convertErrorToString(GLenum errorCode)
    {
        return reinterpret_cast<const char*>(gluErrorString(errorCode));
    }

    void logOpenglDebugMessageStr(std::string file,size_t location, std::string error)
    {
        Logger::logError("[Repeater] OpenGL error:", error, " at ",file,":",location);\
    }

    void logTrace(const std::string msg)
    {
        glGetUniformLocation(0, ("trace: "+msg).c_str());
        CLEAR_GL_ERROR();
    }
}
