/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/opengl_state.cpp
*
*****************************************************************************/

#include "utils/opengl_state.hpp"

namespace ve::utils
{
BackupOpenGLStatesRAII::~BackupOpenGLStatesRAII()
{
    if (m_State)
    {
        if (m_IsEnabled)
        {
            glEnable(m_State);
        }
        else
        {
            glDisable(m_State);
        }
    }
}
BackupOpenGLStatesRAII::BackupOpenGLStatesRAII(GLenum state)
    : m_State(state)
{
    m_IsEnabled = glIsEnabled(state);
}

// Move constructable
BackupOpenGLStatesRAII::BackupOpenGLStatesRAII(BackupOpenGLStatesRAII&& inst)
{
    *this = std::move(inst);
}
// Move assingnable
BackupOpenGLStatesRAII& BackupOpenGLStatesRAII::operator=(BackupOpenGLStatesRAII&& inst)
{
    std::swap(m_State, inst.m_State);
    std::swap(m_IsEnabled, inst.m_IsEnabled);
    return *this;
}

void restoreStateFunctor(const std::vector<GLenum>& states, const std::function<void()>& functor)
{
    std::vector<BackupOpenGLStatesRAII> stateKeepers;
    for (auto& state : states)
    {
        stateKeepers.emplace_back(std::move(BackupOpenGLStatesRAII(state)));
    }
    functor();
}
}
