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
namespace trackers 
{
    class TextureMetadata;

    struct FramebufferAttachment
    {
        GLenum type;
        size_t level = 0;
        size_t layer = 0;
        std::shared_ptr<TextureMetadata> texture;
    };

    struct FramebufferMetadata
    {
        explicit FramebufferMetadata(size_t id);
        void attach(GLenum attachmentType, std::shared_ptr<TextureMetadata> texture, GLenum type = GL_COLOR_ATTACHMENT0, size_t level = 0, size_t layer = 0);
        bool hasAttachment(GLenum attachmentType) const;

        bool hasFailedToCreateShadowFBO() const;
        bool hasShadowFBO() const;
        size_t getShadowFBO() const;

        void setDrawBuffers(std::vector<GLenum> buffers);

        /// Create multi-layer shadow FBO
        void createShadowedFBO(size_t numLayers);

        void freeShadowedFBO();

        /// Create proxy FBO from texture views to a single layer of shadow FBO
        GLuint createProxyFBO(size_t layer);
        /*
         * Heuristics
         */
        /// Has only depth buffer attachment
        bool isShadowMapFBO() const;

        /// Is layered
        bool isLayeredRendering() const;

        /// Has attachment of CUBEMAP type
        bool isEnvironmentMapFBO() const;

        /// Has any attachment
        bool hasAnyAttachment() const;

        ContextTracker<FramebufferAttachment>& getAttachmentMap();

        static std::string getAttachmentTypeAsString(GLenum attachmentType);

        private:
        size_t m_id;
        /// List of intercepted allowed colour attachments (glDrawBuffers), 0 by default
        std::vector<GLenum> m_drawBuffers = {GL_COLOR_ATTACHMENT0};
        ContextTracker<FramebufferAttachment> m_attachments;
        std::vector<utils::FBORAII> m_proxyFBO;

        size_t m_shadowFBOId = 0;
        bool m_hasCreationOfShadowFBOFailed = false;
    };

    class FramebufferTracker: public BindableContextTracker<std::shared_ptr<FramebufferMetadata>>
    {
        public:
        // Heuristics
        bool isFBODefault() const;
        bool isSuitableForRepeating() const;
    };
} // namespace trackers
} // namespace ve
#endif
