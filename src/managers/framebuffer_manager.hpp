#ifndef VE_FRAMEBUFFER_MANAGER_HPP
#define VE_FRAMEBUFFER_MANAGER_HPP

#include <functional>

namespace ve
{
    class Context;
}

namespace managers
{
    class FramebufferManager
    {
        public:
        void glMatrixMode(Context&, GLenum mode);
        void glLoadMatrixd(Context&, const GLdouble* m);
        void glLoadMatrixf(Context&, const GLfloat* m);
        void glLoadIdentity(Context&, void);
        void glMultMatrixd(Context&, const GLdouble* m);
        void glMultMatrixf(Context&, const GLfloat* m);
        void glOrtho(Context&, GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val);
        void glFrustum(Context&, GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val);
    }
}
}
#endif
