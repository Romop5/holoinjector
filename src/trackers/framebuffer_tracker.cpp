#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

#include "trackers/framebuffer_tracker.hpp"
#include "trackers/texture_tracker.hpp"

#include <cassert>
#include <unordered_set>

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

size_t ve::FramebufferMetadata::getShadowFBO() const
{
    return m_shadowFBOId;
}

void ve::FramebufferMetadata::createShadowedFBO()
{
    GLuint shadowFBO;
    glGenFramebuffers(1,&shadowFBO);
    // Hack: OpenGL require at least one bind before attaching
    glBindFramebuffer(GL_FRAMEBUFFER,shadowFBO);
    for(auto& [attachmentType, metadata]: m_attachments.getMap())
    {
        auto texture = metadata.texture;
        if(!texture->hasShadowTexture())
        {
            texture->createShadowedTexture();
        }
        auto shadowedTexture = texture->getShadowedTextureId();
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, shadowedTexture,metadata.level);
    }
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    assert(status == GL_FRAMEBUFFER_COMPLETE);
    m_shadowFBOId = shadowFBO;
}

GLuint ve::FramebufferMetadata::createProxyFBO(size_t layer)
{
    GLuint proxyFBO;
    glGenFramebuffers(1,&proxyFBO);
    // Hack: OpenGL require at least one bind before attaching
    glBindFramebuffer(GL_FRAMEBUFFER,proxyFBO);
    for(auto& [attachmentType, metadata]: m_attachments.getMap())
    {
        auto texture = metadata.texture;
        if(!texture->hasShadowTexture())
        {
            texture->createShadowedTexture();
        }
        auto shadowedTexture = texture->getShadowedTextureId();
        glFramebufferTexture3D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D_ARRAY,shadowedTexture,metadata.level, layer);
    }
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    assert(status == GL_FRAMEBUFFER_COMPLETE);
    return proxyFBO;
}


bool ve::FramebufferMetadata::isShadowMapFBO() const
{
    return !m_attachments.has(GL_COLOR_ATTACHMENT0) && 
            (m_attachments.has(GL_DEPTH_ATTACHMENT) || m_attachments.has(GL_DEPTH_STENCIL_ATTACHMENT)) &&
            m_attachments.size() == 1;
}
bool ve::FramebufferMetadata::isEnvironmentMapFBO() const
{
    const static auto cubeMapTypes = std::unordered_set<GLenum>{GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP,GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_PROXY_TEXTURE_CUBE_MAP};

    if(m_attachments.has(GL_DEPTH_ATTACHMENT))
    {
        auto type = m_attachments.getConst(GL_DEPTH_ATTACHMENT).texture->getType();
        if(cubeMapTypes.count(type) > 0)
            return true;
    }

    if(m_attachments.has(GL_COLOR_ATTACHMENT0))
    {
        auto type = m_attachments.getConst(GL_COLOR_ATTACHMENT0).texture->getType();
        if(cubeMapTypes.count(type) > 0)
            return true;
    }
    return false;
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


