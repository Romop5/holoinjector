#include "framebuffer_manager.hpp"

using namespace ve;
using namespace ve::managers;

void FramebufferManager::glMatrixMode(Context& context, GLenum mode)
{
    glMatrixMode(mode);
    m_Context.getLegacyTracker().matrixMode(mode);
}
void FramebufferManager::glLoadMatrixd(Context& context, const GLdouble* m)
{
    glLoadMatrixd(m);
    if(m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}
void FramebufferManager::glLoadMatrixf(Context& context, const GLfloat* m)
{
    glLoadMatrixf(m);
    if(m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
    //helper::dumpOpenglMatrix(m);
}

void FramebufferManager::glLoadIdentity(Context& context, void)
{
    glLoadIdentity();
    if(m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        glm::mat4 identity = glm::mat4(1.0);
        m_Context.getLegacyTracker().loadMatrix(std::move(identity));
    }
}

void FramebufferManager::glMultMatrixd(Context& context, const GLdouble* m)
{
    glMultMatrixd(m);
    if(m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
}

void FramebufferManager::glMultMatrixf(Context& context, const GLfloat* m)
{
    glMultMatrixf(m);
    if(m_Context.getLegacyTracker().getMatrixMode() == GL_PROJECTION)
    {
        const auto result = opengl_utils::createMatrixFromRawGL(m);
        m_Context.getLegacyTracker().loadMatrix(std::move(result));
    }
}


void FramebufferManager::glOrtho(Context& context, GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val)
{
    glOrtho(left,right,bottom,top,near_val,far_val);
    m_Context.getLegacyTracker().multMatrix(glm::ortho(left,right,bottom,top,near_val,far_val));
}

void FramebufferManager::glFrustum(Context& context, GLdouble left,GLdouble right,GLdouble bottom,GLdouble top,GLdouble near_val,GLdouble far_val)
{
    glFrustum(left,right,bottom,top,near_val,far_val);
    m_Context.getLegacyTracker().multMatrix(glm::frustum(left,right,bottom,top,near_val,far_val));
}
