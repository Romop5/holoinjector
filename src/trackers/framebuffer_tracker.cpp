#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "trackers/framebuffer_tracker.hpp"
#include "trackers/texture_tracker.hpp"

#include "utils/opengl_debug.hpp"

#include <cassert>
#include <unordered_set>

using namespace ve;
using namespace ve::trackers;

///////////////////////////////////////////////////////////////////////////////
// FramebufferMetadata 
///////////////////////////////////////////////////////////////////////////////
void ve::trackers::FramebufferMetadata::attach(GLenum attachmentType, std::shared_ptr<TextureMetadata> texture, GLenum type, size_t level, size_t layer)
{
    FramebufferAttachment attachment;
    attachment.type = type;
    attachment.level = level;
    attachment.layer = layer;
    attachment.texture = texture;
    m_attachments.add(attachmentType, attachment);

    assert(hasAnyAttachment() == true);
    assert(hasAttachment(attachmentType) == true);
}

bool ve::trackers::FramebufferMetadata::hasAttachment(GLenum attachmentType) const
{
    return m_attachments.has(attachmentType);
}

bool ve::trackers::FramebufferMetadata::hasShadowFBO() const
{
    return m_shadowFBOId;
}

size_t ve::trackers::FramebufferMetadata::getShadowFBO() const
{
    return m_shadowFBOId;
}

void ve::trackers::FramebufferMetadata::createShadowedFBO(size_t numLayers)
{
    assert(hasAnyAttachment());
    ASSERT_GL_ERROR();
    GLuint shadowFBO;
    glGenFramebuffers(1,&shadowFBO);
    ASSERT_GL_ERROR();
    ASSERT_GL_NEQ(shadowFBO,0);
    // Hack: OpenGL require at least one bind before attaching
    glBindFramebuffer(GL_FRAMEBUFFER,shadowFBO);
    ASSERT_GL_ERROR();
    for(auto& [attachmentType, metadata]: m_attachments.getMap())
    {
        auto texture = metadata.texture;
        if(!texture->hasShadowTexture())
        {
            texture->createShadowedTexture(numLayers);
            if(!texture->hasShadowTexture())
            {
                Logger::logError("[Repeater] Failed to create shadow texture for FBO. ", ENHANCER_POS);
                return;
            }
        }
        auto shadowedTexture = texture->getShadowedTextureId();
        ASSERT_GL_NEQ(shadowedTexture,0);
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, shadowedTexture,metadata.level);
        assert(glGetError() == GL_NO_ERROR);
    }
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_EQ(status,GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    if(status == GL_FRAMEBUFFER_COMPLETE)
    {
        m_shadowFBOId = shadowFBO;
    } else {
        std::string attachmentsDump;
        for(auto& [attachmentType, metadata]: m_attachments.getMap())
        {
            attachmentsDump += (" ") + FramebufferMetadata::getAttachmentTypeAsString
                (attachmentType);
        }
        Logger::logError("Failed to create shadow FBO. Count of attachments: ", m_attachments.size(), "\n",
                attachmentsDump);
        glDeleteFramebuffers(1, &shadowFBO);
    }
}

GLuint ve::trackers::FramebufferMetadata::createProxyFBO(size_t layer)
{
    // draw() call should be over existing FBO, which should have been previously created
    // and binded, thus a shadow FBO must already exist!
    assert(hasShadowFBO());

        // Use cached FBO if available
    if(layer < m_proxyFBO.size() && m_proxyFBO[layer].getID() != 0)
    {
        return m_proxyFBO[layer].getID();
    }

    Logger::logDebug("[Repeater] proxy FBO for layer ",layer," not found in cache -> creating");
    Logger::logDebug("[Repeater] cache size:", m_proxyFBO.size(), " elements");

    m_proxyFBO.resize(layer+1);

    GLuint proxyFBO;
    glGenFramebuffers(1,&proxyFBO);
    ASSERT_GL_ERROR();
    // Hack: OpenGL require at least one bind before attaching
    glBindFramebuffer(GL_FRAMEBUFFER,proxyFBO);
    ASSERT_GL_ERROR();
    for(auto& [attachmentType, metadata]: m_attachments.getMap())
    {
        auto texture = metadata.texture;
        // This should hold, because we call createShadowedFBO() above
        assert(texture->hasShadowTexture());
        auto shadowedTexture = texture->getShadowedTextureId();
        assert(shadowedTexture != 0);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentType, shadowedTexture,metadata.level, layer);
        ASSERT_GL_ERROR();
    }
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_EQ(status, GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    ASSERT_GL_ERROR();
    Logger::logDebug("[Repeater] storing proxy FBO for layer ",layer);
    m_proxyFBO[layer] = std::move(utils::FBORAII(proxyFBO));
    Logger::logDebug("[Repeater] post cache size:", m_proxyFBO.size(), " elements");
    return proxyFBO;
}


bool ve::trackers::FramebufferMetadata::isShadowMapFBO() const
{
    return !m_attachments.has(GL_COLOR_ATTACHMENT0) && 
            (m_attachments.has(GL_DEPTH_ATTACHMENT) || m_attachments.has(GL_DEPTH_STENCIL_ATTACHMENT)) &&
            m_attachments.size() == 1;
}

bool ve::trackers::FramebufferMetadata::isLayeredRendering() const
{
    return m_attachments.has(GL_COLOR_ATTACHMENT0) && m_attachments.has(GL_COLOR_ATTACHMENT1);
}
bool ve::trackers::FramebufferMetadata::isEnvironmentMapFBO() const
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

bool ve::trackers::FramebufferMetadata::hasAnyAttachment() const
{
    return m_attachments.size() > 0;
}

ContextTracker<FramebufferAttachment>& ve::trackers::FramebufferMetadata::getAttachmentMap()
{
    return m_attachments;
}

std::string ve::trackers::FramebufferMetadata::getAttachmentTypeAsString(GLenum attachmentType)
{
    switch(attachmentType)
    {
        case GL_COLOR_ATTACHMENT0: return "GL_COLOR_ATTACHMENT0";
        case GL_COLOR_ATTACHMENT1: return "GL_COLOR_ATTACHMENT1";
        case GL_COLOR_ATTACHMENT2: return "GL_COLOR_ATTACHMENT2";
        case GL_COLOR_ATTACHMENT3: return "GL_COLOR_ATTACHMENT3";
        case GL_COLOR_ATTACHMENT4: return "GL_COLOR_ATTACHMENT4";
        case GL_COLOR_ATTACHMENT5: return "GL_COLOR_ATTACHMENT5";
        case GL_COLOR_ATTACHMENT6: return "GL_COLOR_ATTACHMENT6";
        case GL_COLOR_ATTACHMENT7: return "GL_COLOR_ATTACHMENT7";
        case GL_COLOR_ATTACHMENT8: return "GL_COLOR_ATTACHMENT8";
        case GL_DEPTH_ATTACHMENT: return "GL_DEPTH_ATTACHMENT";
        case GL_STENCIL_ATTACHMENT: return "GL_STENCIL_ATTACHMENT";
        case GL_DEPTH_STENCIL_ATTACHMENT: return "GL_DEPTH_STENCIL_ATTACHMENT";
        default:
            return "UNKNOWN_ATTACHMENT"+std::to_string(attachmentType);
    }
}
///////////////////////////////////////////////////////////////////////////////
// FramebufferTracker
///////////////////////////////////////////////////////////////////////////////
bool ve::trackers::FramebufferTracker::isFBODefault() const
{
    return getBoundId() == 0;
}

bool ve::trackers::FramebufferTracker::isSuitableForRepeating() const
{
    if(isFBODefault())
        return true;
    auto fbo = getBoundConst();
    return !fbo->isShadowMapFBO() && !fbo->isEnvironmentMapFBO();
}


