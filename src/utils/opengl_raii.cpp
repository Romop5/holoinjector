#define GL_GLEXT_PROTOTYPES 1
#include "utils/opengl_raii.hpp"
#include "GL/glext.h"

ve::utils::FBORAII::~FBORAII()
{
    if(m_id)
        glDeleteFramebuffers(1, &m_id);
    m_id = 0;
}

