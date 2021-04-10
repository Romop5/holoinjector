/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_state.hpp
*
*****************************************************************************/

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
        explicit BackupOpenGLStatesRAII(GLenum state);
        ~BackupOpenGLStatesRAII();

        // Delete copy constructor
        BackupOpenGLStatesRAII(const BackupOpenGLStatesRAII&) = delete;
        // Delete copy assign
        BackupOpenGLStatesRAII& operator=(const BackupOpenGLStatesRAII&) = delete;

        // Move constructable
        BackupOpenGLStatesRAII(BackupOpenGLStatesRAII&& inst);
        // Move assingnable
        BackupOpenGLStatesRAII& operator=(BackupOpenGLStatesRAII&& inst);

        private:
        GLenum m_State = GL_FALSE;
        bool m_IsEnabled = false;
    };

    void restoreStateFunctor(const std::vector<GLenum>& states, const std::function<void()>& functor);

} // namespace raii
} // namespace ve
#endif



