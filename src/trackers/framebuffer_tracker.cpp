#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

#include "trackers/framebuffer_tracker.hpp"
#include "trackers/texture_tracker.hpp"

#include <cassert>

using namespace ve;

///////////////////////////////////////////////////////////////////////////////
// FramebufferMetadata 
///////////////////////////////////////////////////////////////////////////////
void ve::FramebufferMetadata::attach(GLenum attachmentType, std::shared_ptr<TextureMetadata> texture, FramebufferAttachment::DType type, size_t level, size_t layer)
{
    FramebufferAttachment attachment;
    attachment.type = type;
    attachment.level = level;
    attachment.layer = layer;
    attachment.texture = texture;
    m_attachments.add(attachmentType, attachment);
}

bool ve::FramebufferMetadata::hasAttachment(GLenum attachmentType) const
{
    return m_attachments.has(attachmentType);
}

bool ve::FramebufferMetadata::hasShadowFBO() const
{
    return m_shadowFBOId;
}
void ve::FramebufferMetadata::createShadowedFBO()
{
    GLuint shadowFBO;
    glGenFramebuffers(1,&shadowFBO);
    for(auto& [attachmentType, metadata]: m_attachments.getMap())
    {
        auto texture = metadata.texture;
        if(!texture->hasShadowTexture())
        {
            texture->createShadowedTexture();
        }
        auto shadowedTexture = texture->getShadowedTextureId();
        glNamedFramebufferTexture(shadowFBO, attachmentType, shadowedTexture,metadata.level);
    }
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);
    m_shadowFBOId = shadowFBO;
}

bool ve::FramebufferMetadata::isShadowMapFBO() const
{
    return !m_attachments.has(GL_COLOR_ATTACHMENT0) && 
            (m_attachments.has(GL_DEPTH_ATTACHMENT) || m_attachments.has(GL_DEPTH_STENCIL_ATTACHMENT)) &&
            m_attachments.size() == 1;
}
bool ve::FramebufferMetadata::isEnvironmentMapFBO() const
{
    return m_attachments.has(GL_DEPTH_ATTACHMENT) &&
           m_attachments.getConst(GL_DEPTH_ATTACHMENT).texture->getType() == GL_TEXTURE_CUBE_MAP;
}

///////////////////////////////////////////////////////////////////////////////
// FramebufferTracker
///////////////////////////////////////////////////////////////////////////////
bool ve::FramebufferTracker::isFBODefault() const
{
    return getBoundId() == 0;
}

bool ve::FramebufferTracker::isSuitableForRepeating() const
{
    if(isFBODefault())
        return true;
    auto fbo = getBoundConst();
    return !fbo->isShadowMapFBO() && !fbo->isEnvironmentMapFBO();
}


