#include "utils/opengl_state.hpp"

namespace ve::utils
{
    BackupOpenGLStatesRAII::~BackupOpenGLStatesRAII()
    {
        if(m_IsEnabled)
        {
            glEnable(m_State);
        } else {
            glDisable(m_State);
        }
    }
    BackupOpenGLStatesRAII::BackupOpenGLStatesRAII(GLenum state)
        : m_State(state)
    {
        m_IsEnabled = glIsEnabled(state);
    }

    void restoreStateFunctor(const std::vector<GLenum>& states, const std::function<void()>& functor)
    {
        std::vector<BackupOpenGLStatesRAII> stateKeepers;
        for(auto& state: states)
        {
            stateKeepers.push_back(BackupOpenGLStatesRAII(state));
        }
        functor();
    }
}
