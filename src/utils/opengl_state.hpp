#ifndef VE_UTILS_OPENGL_STATE_HPP
#define VE_UTILS_OPENGL_STATE_HPP

#include <GL/gl.h>
#include <vector>
#include <functional>

namespace ve
{
namespace utils
{
    /**
     * \brief RAII helper which restores original state of OpenGL on end
     */
    class BackupOpenGLStatesRAII
    {
        public:
        BackupOpenGLStatesRAII() = default;
        ~BackupOpenGLStatesRAII();
        explicit BackupOpenGLStatesRAII(GLenum state);
        private:
        GLenum m_State;
        bool m_IsEnabled = false;
    };

    void restoreStateFunctor(const std::vector<GLenum>& states, const std::function<void()>& functor);

} // namespace raii
} // namespace ve
#endif



