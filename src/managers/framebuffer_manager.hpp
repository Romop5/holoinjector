#ifndef VE_FRAMEBUFFER_MANAGER_HPP
#define VE_FRAMEBUFFER_MANAGER_HPP

#include <functional>

namespace ve
{
    class Context;
namespace managers
{
    class FramebufferManager
    {
        public:
        void clear(Context& context, GLbitfield mask);
        void bindFramebuffer (Context& context, GLenum target, GLuint framebuffer);
        void swapBuffers(Context& context, std::function<void(void)> swapit, std::function<void(void)> onOverlayRender);
    };
}
}
#endif
