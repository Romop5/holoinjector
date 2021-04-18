/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_raii.hpp
*
*****************************************************************************/

#ifndef HI_UTILS_OPENGL_RAII_HPP
#define HI_UTILS_OPENGL_RAII_HPP

#include "utils/opengl_objects.hpp"

namespace hi
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
} // namespace hi
#endif
