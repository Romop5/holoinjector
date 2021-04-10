/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_raii.hpp
*
*****************************************************************************/

#ifndef VE_UTILS_OPENGL_RAII_HPP
#define VE_UTILS_OPENGL_RAII_HPP

#include "utils/opengl_objects.hpp"

namespace ve
{
namespace utils
{
    /// FBO RAII owner
    struct FBORAII : public glObject
    {
        FBORAII() = default;
        explicit FBORAII(GLuint fboId)
            : glObject(fboId)
        {
        }

    private:
    };
} // namespace raii
} // namespace ve
#endif
