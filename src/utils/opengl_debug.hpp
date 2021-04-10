/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_debug.hpp
*
*****************************************************************************/

#ifndef OPENGL_DEBUG_HPP
#define OPENGL_DEBUG_HPP

#include "GL/gl.h"
#include "logger.hpp"
#include <string>

#define CLEAR_GL_ERROR() \
    glGetError();

#define ASSERT_GL_ERROR()                                                \
    {                                                                    \
        auto error = glGetError();                                       \
        if (error != GL_NO_ERROR)                                        \
        {                                                                \
            ve::debug::logOpenglDebugMessage(__FILE__, __LINE__, error); \
            assert(false);                                               \
        }                                                                \
    }

#define ASSERT_GL_EQ(term1, term2) ASSERT_GL(term1, term2, ==)
#define ASSERT_GL_NEQ(term1, term2) ASSERT_GL(term1, term2, !=)

#define ASSERT_GL(term1, term2, op)                                                              \
    {                                                                                            \
        if (!(term1 op term2))                                                                   \
        {                                                                                        \
            std::string expand1 = std::string("" #term1) + "(" + Logger::ToString(term1) + ")";  \
            std::string expand2 = std::string("" #term2) + "(" + Logger::ToString(term2) + ")";  \
            ve::debug::logOpenglDebugMessageStr(__FILE__, __LINE__, expand1 + " vs " + expand2); \
            assert(term1 == term2);                                                              \
        }                                                                                        \
    }

namespace ve
{
namespace debug
{
    void logOpenglDebugMessage(std::string file, size_t location, GLenum errorCode);
    void logOpenglDebugMessageStr(std::string file, size_t location, std::string error);
    void logTrace(const std::string msg);
    std::string convertErrorToString(GLenum errorCode);
}
}

#endif
