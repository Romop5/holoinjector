#ifndef VE_FRAMEBUFFER_TRACKER_HPP
#define VE_FRAMEBUFFER_TRACKER_HPP

#include <vector>
#include <unordered_map>
#include <memory>

#include <GL/gl.h>
#include "utils/context_tracker.hpp"
#include "utils/opengl_raii.hpp"


namespace ve
{
    /// 
    class TextureMetadata;

    struct FramebufferAttachment
    {
        enum DType
        {
            ATTACHMENT_0D,
            ATTACHMENT_1D,
            ATTACHMENT_2D,
            ATTACHMENT_3D,
        } type;
        size_t level = 0;
        size_t layer = 0;
        std::shared_ptr<TextureMetadata> texture;
    };

    struct FramebufferMetadata
    {
        void attach(GLenum attachmentType, std::shared_ptr<TextureMetadata> texture, FramebufferAttachment::DType type = FramebufferAttachment::DType::ATTACHMENT_0D, size_t level = 0, size_t layer = 0);
        bool hasAttachment(GLenum attachmentType) const;

        bool hasShadowFBO() const;
        size_t getShadowFBO() const;

        /// Create multi-layer shadow FBO
        void createShadowedFBO();

        /// Create proxy FBO from texture views to a single layer of shadow FBO
        GLuint createProxyFBO(size_t layer);
        /*
         * Heuristics
         */
        /// Has only depth buffer attachment
        bool isShadowMapFBO() const;
        /// Has attachment of CUBEMAP type
        bool isEnvironmentMapFBO() const;

        /// Has any attachment
        bool hasAnyAttachment() const;
        private:
        ContextTracker<FramebufferAttachment> m_attachments;
        std::vector<utils::FBORAII> m_proxyFBO;

        size_t m_shadowFBOId = 0;
    };

    class FramebufferTracker: public BindableContextTracker<std::shared_ptr<FramebufferMetadata>>
    {
        public:
        // Heuristics
        bool isFBODefault() const;
        bool isSuitableForRepeating() const;
    };
} // namespace ve
#endif
