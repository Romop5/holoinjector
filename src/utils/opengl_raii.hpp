#ifndef VE_UTILS_OPENGL_RAII_HPP
#define VE_UTILS_OPENGL_RAII_HPP

#include "GL/gl.h"

namespace ve
{
namespace utils
{
    /// FBO RAII owner
    struct FBORAII
    {
        FBORAII() = default;
        explicit FBORAII(GLuint fboId): m_id(fboId) {}
        FBORAII(FBORAII&&) = default;
        FBORAII& operator=(FBORAII&&) = default;
        ~FBORAII();
        GLuint getID() const { return m_id; }
        private:
        GLuint m_id = 0;
    };
} // namespace raii
} // namespace ve
#endif
