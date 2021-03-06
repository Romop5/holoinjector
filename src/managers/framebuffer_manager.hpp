/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        managers/framebuffer_manager.hpp
*
*****************************************************************************/

#ifndef HI_FRAMEBUFFER_MANAGER_HPP
#define HI_FRAMEBUFFER_MANAGER_HPP

#include <functional>

namespace hi
{
class Context;
namespace managers
{
    class FramebufferManager
    {
    public:
        void clear(Context& context, GLbitfield mask);
        void bindFramebuffer(Context& context, GLenum target, GLuint framebuffer);
        void swapBuffers(Context& context, std::function<void(void)> swapit);
        void renderFromOutputFBO(Context& context);
    };
}
}
#endif
