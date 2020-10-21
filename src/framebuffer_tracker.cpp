#include "framebuffer_tracker.hpp"

using namespace ve;

void ve::FramebufferTracker::attach(GLenum attachment, GLuint texture)
{
   switch(attachment)
    {
        case GL_DEPTH_ATTACHMENT:
            attachDepth(getBoundId());
            break;
        case GL_STENCIL_ATTACHMENT:
            attachStencil(getBoundId());
            break;
        case GL_DEPTH_STENCIL_ATTACHMENT:
            attachDepth(getBoundId());
            attachStencil(getBoundId());
            break;
        case GL_COLOR_ATTACHMENT0:
        case GL_COLOR_ATTACHMENT1:
        case GL_COLOR_ATTACHMENT2:
        case GL_COLOR_ATTACHMENT3:
        case GL_COLOR_ATTACHMENT4:
        case GL_COLOR_ATTACHMENT5:
        case GL_COLOR_ATTACHMENT6:
        case GL_COLOR_ATTACHMENT7:
        case GL_COLOR_ATTACHMENT8:
            attachColor(getBoundId());
            break;
    }
}

void ve::FramebufferTracker::attachDepth(size_t frameBuffer)
{
    get(frameBuffer)->hasDepthBufferAttached = true;
}

void ve::FramebufferTracker::attachColor(size_t frameBuffer)
{
    get(frameBuffer)->numberOfColorBuffersAttached++;
}

void ve::FramebufferTracker::attachStencil(size_t frameBuffer)
{
    get(frameBuffer)->hasStencil = true;
}


bool ve::FramebufferTracker::isFBODefault() const
{
    return getBoundId() == 0;
}
bool ve::FramebufferTracker::isFBOshadowMap() const
{
    if(isFBODefault())
        return false;
    const auto& fbo = getConst(getBoundId());
    return fbo->hasDepthBufferAttached && fbo->numberOfColorBuffersAttached == 0;
}

