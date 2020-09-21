#include "framebuffer_tracker.hpp"

using namespace ve;

void ve::FramebufferTracker::addFramebuffer(size_t id)
{
    m_buffers[id] = FramebufferMetadata();
}

bool ve::FramebufferTracker::hasFramebuffer(size_t id) const
{
    return m_buffers.count(id) > 0;
}

void ve::FramebufferTracker::deteteFramebuffer(size_t id)
{
    m_buffers.erase(id);
}

void ve::FramebufferTracker::attach(GLenum attachment, GLuint texture)
{
   switch(attachment)
    {
        case GL_DEPTH_ATTACHMENT:
            attachDepth(m_currentFramebuffer);
            break;
        case GL_STENCIL_CLEAR_VALUE:
            attachStencil(m_currentFramebuffer);
            break;
        case GL_COLOR_ATTACHMENT0:
            attachColor(m_currentFramebuffer);
            break;
    }
}

void ve::FramebufferTracker::attachDepth(size_t frameBuffer)
{
    m_buffers[frameBuffer].hasDepthBufferAttached = true;
}

void ve::FramebufferTracker::attachColor(size_t frameBuffer)
{
    m_buffers[frameBuffer].numberOfColorBuffersAttached++;
}

void ve::FramebufferTracker::attachStencil(size_t frameBuffer)
{
    m_buffers[frameBuffer].hasStencil = true;
}


void ve::FramebufferTracker::bind(size_t id)
{
    m_currentFramebuffer = id; 
}

size_t ve::FramebufferTracker::getCurrentFrameBuffer() const
{
    return m_currentFramebuffer;
}

bool ve::FramebufferTracker::isFBODefault() const
{
    return m_currentFramebuffer == 0;
}
bool ve::FramebufferTracker::isFBOshadowMap() const
{
    if(isFBODefault())
        return false;
    const auto& fbo = m_buffers.at(m_currentFramebuffer);
    return fbo.hasDepthBufferAttached && fbo.numberOfColorBuffersAttached == 0;
}

