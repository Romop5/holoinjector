/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        trackers/framebuffer_tracker.hpp
*
*****************************************************************************/

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

    /**
     * \brief Tracks a single attachment to FBO
     */
    struct FramebufferAttachment
    {
        /// Type of attachment (e.g. GL_COLOR_ATTACHMENT0)
        GLenum type;
        /// TODO: remove (redundant)
        size_t level = 0;
        /// TODO: remove (redundant)
        size_t layer = 0;
        /// Link to the texture object, which tracks the original texture
        std::shared_ptr<TextureMetadata> texture;
    };

    /**
     * \brief Tracks metadata of a single Opengl's Framebuffer Objects (FBO)
     *
     * For FBO, we need to track type of attachements to decide if it's worth to
     * render multiviewd to such FBO.
     * In addition, we also keep track of additional internal structures, used for injection.
     *
     * ## Concept of Shadow FBO
     * In order to render scenes in multi-view mode, a shadow FBO is created behind-the-scenes to 
     * store results of each individual view of multi-view render.
     * Later, when application render to original FBO, the shadowed FBO is transparently binded
     * instead to support replication of geometry to multiple output views.
     *
     * ## Proxy FBO
     * If shader program does not support GS instancing or does not have Geometry Shader itself, one
     * can't simply render to a layered FBO in a single draw call.
     * Instead, it's possible to create *proxy FBO*, a FBO made of texture views, where each texture
     * view is a proxy texture to a certain layer of original attachment's texture.
     * createProxyFBO() thus serves to create a new FBO, which points to a certain layer of shadowed FBO.
     */
    struct FramebufferMetadata
    {
        explicit FramebufferMetadata(size_t id);
        /// Store attachment
        void attach(GLenum attachmentType, std::shared_ptr<TextureMetadata> texture, GLenum type = GL_COLOR_ATTACHMENT0, size_t level = 0, size_t layer = 0);

        /// Look up if there is any attachemtn of attachmentType
        bool hasAttachment(GLenum attachmentType) const;

        /// Store in which colour buffers should FBO draw to
        void setDrawBuffers(std::vector<GLenum> buffers);

        /*
         * Shadow FBO related functions
         * TODO: refactor the methods for shadow FBO's manipulation to a separate class to follow
         * single responsibility principle pattern.
         */
        /// Determine if shadow FBO has already been attempted to be created
        bool hasFailedToCreateShadowFBO() const;

        /// Determine if shadow FBO has been created
        bool hasShadowFBO() const;
        /// Get shadow FBO's OpenGL's object ID. Undefined behavior if shadow FBO does not exist
        size_t getShadowFBO() const;

        /// Create multi-layer shadow FBO
        void createShadowedFBO(size_t numLayers);

        /// Delete all shadow FBO relatd resources
        void freeShadowedFBO();

        /// Create proxy FBO from texture views to a single layer of shadow FBO
        GLuint createProxyFBO(size_t layer);
        /*
         * Heuristics
         */
        /// Has only depth buffer attachment without any output colour buffers
        bool isShadowMapFBO() const;

        /// Is layered (thus has more than single colour output buffer)
        bool isLayeredRendering() const;

        /// Has attachment of CUBEMAP type
        bool isEnvironmentMapFBO() const;

        /// Has any attachment
        bool hasAnyAttachment() const;

        ContextTracker<FramebufferAttachment>& getAttachmentMap();

        /// Debug: serialize attachment type to string
        static std::string getAttachmentTypeAsString(GLenum attachmentType);

        private:
        /// Tracked FBO's OpenGL object ID
        size_t m_id;
        /// Vector of intercepted allowed colour attachments (glDrawBuffers), 0 by default
        std::vector<GLenum> m_drawBuffers = {GL_COLOR_ATTACHMENT0};
        /// Vector of attachments
        ContextTracker<FramebufferAttachment> m_attachments;
        /**
         * \brief Cache of proxy FBOs for current shadow FBO
         *
         * Instead of creating proxy FBOs in each frame again, FBOs are only create once (at the
         * first time) and then m_proxyFBO is used as a Look-Up Table
         */
        std::vector<utils::FBORAII> m_proxyFBO;

        /// Shadow FBO's ID
        size_t m_shadowFBOId = 0;

        /**
         * \brief Flag: has creation of shadow FBO failed?
         *
         * This serves as a cache of initialization process to prevent repetitive time-consuming constructions
         * when we already know that the creation fails for current FBO.
         */
        bool m_hasCreationOfShadowFBOFailed = false;
    };

    /**
     * \brief Tracks creation & binding of Opengl's Framebuffer Objects (FBO)
     */
    class FramebufferTracker: public BindableContextTracker<std::shared_ptr<FramebufferMetadata>>
    {
        public:
        /*
         * Heuristics
         */
        /// Is currently bound FBO the default one (ID == 0)?
        bool isFBODefault() const;
        /// Is currently bound FBO suitable for injection?
        bool isSuitableForRepeating() const;
    };
} // namespace trackers
} // namespace ve
#endif
