#ifndef VE_TEXTURE_TRACKER_HPP
#define VE_TEXTURE_TRACKER_HPP
#include <memory>
#include "GL/gl.h"

#include "utils/context_tracker.hpp"
#include "pipeline/program_metadata.hpp"

namespace ve
{
namespace trackers
{
    /**
     * \brief Internal: distinquish between texture and renderbuffer
     */
    enum class TextureType
    {
        TEXTURE,
        RENDERBUFFER
    };

    /**
     * \brief Tracks metadata for each texture
     *
     * ## Shadowing
     * In combination with shadowed FBO, described in FramebufferTracker's documentation, textures
     * have to support shadowing as well in order to create multi-layered rendering framebuffer
     * objects.
     * Each texture can be thus shadowed by creating a multi-layer texture (GL_TEXTURE_2D_ARRAY)
     * and rebinding texture units to shadow texture.
     *
     * ## Texture views
     * To obtain a single GL_TEXTURE_2D for layered shadow texture, texture views are used to create proxy
     * textures, pointing to precise layer of shadow texture.
     */
    class TextureMetadata
    {
        public:
        TextureMetadata(size_t id): m_Id(id) {}
        virtual ~TextureMetadata();
        void deinitialize();

        /// Determine if texture is renderbuffer or texture
        virtual TextureType getPhysicalTextureType();

        /// Keep track of intercepted texture's metadata such as format and resolution
        void setStorage(GLenum type, size_t width, size_t height, size_t levels, size_t layers, GLenum internalFormat);

        /*
         * Queries
         */
        /// Get OpenGL's object ID
        size_t getID();

        size_t getWidth() const;
        size_t getHeight() const;
        size_t getLayers() const;
        size_t getLevels() const;

        /// Get type of texture object (e.g. GL_TEXTURE_2D)
        GLenum getType();
        /// Get internal format of texture (e.g. GL_RGBA8)
        GLenum getFormat();

        /*
         * Texture shadowing facility
         * TODO: refactor to separate class
         */
        bool hasShadowTexture() const;
        size_t getShadowedTextureId() const;
        /// Get the most recently created texture view
        size_t getTextureViewIdOfShadowedTexture() const;

        /// Attempt to create layered (array) version of texture
        virtual void createShadowedTexture(size_t numOfLayers = 9);

        /// Set texture view to precise level of shadowed texture. Undefined behavior if shadow FBO does not exist
        void setTextureViewToLayer(size_t layer);
        void freeShadowedTexture();

        /// Helper: serialize type (e.gl GL_TEXTURE_2D) to string
        static std::string getTypeAsString(GLenum type);

        /// Helper: serialize format (e.gl GL_RGBA8) to string
        static std::string getFormatAsString(GLenum type);
        protected:
        /// Resource's ID
        size_t m_Id = 0;

        /// Texture type (e.g. GL_TEXTURE_2D)
        GLenum m_Type = 0;
        /// Format (e.g. GL_RGBA8)
        GLenum m_Format = 0;
        /// Width (original texture)
        size_t m_Width = 0;
        /// Height (original texture)
        size_t m_Height = 0;
        /// TODO:remove; Mipmap levels (original texture)
        size_t m_Levels = 1;
        /// TODO:remove; Layeres original texture)
        size_t m_Layers = 0;

        /*
         * Shadowing resources
         */
        /// Layered shadow texture resource ID
        size_t m_shadowedLayerVersionId = 0;
        /// Single-layer texture, pointing to a precise layer of shadow texture
        size_t m_shadowTextureViewId = 0;
    };

    /**
     * \brief Map of <texture_type, textureID>
     */
    class TextureUnit: public ContextTracker<std::shared_ptr<TextureMetadata>>
    {
    };

    // Forwarding due to friend class
    class TextureTracker;

    /**
     * \brief Tracks binding of OpenGL's textures to texture units
     * 
     * Such tracking is needed to bind layered (shadowed) textures for active original textures when
     * needed.
     */

    class TextureUnitTracker: private BindableContextTracker<std::shared_ptr<TextureUnit>,false>
    {
        public:
        /// Has any texture with shadow texture active?
        bool hasShadowedTextureBinded() const;

        /// Rebind all texture units with textures with shadow texture to certain layer
        void bindShadowedTexturesToLayer(size_t layer);

        /// Rebind to original (application's) texture
        void unbindShadowedTextures();
        protected:
        /// Track activation of texture unit with id
        void activate(size_t id);

        void bind(size_t target, std::shared_ptr<TextureMetadata> texture);
        MapType& getUnits();
        friend class TextureTracker;
        private:
        using baseType = BindableContextTracker<std::shared_ptr<TextureUnit>,false>;
    };

    /**
     * \brief Tracks creation & binding of Opengl's texture objects
     *
     * In addition to tracking lifetime of textures, texture units are also tracked due to necessity
     * to rebind shadowed textures prior to draw calls.
     */
    class TextureTracker: public ContextTracker<std::shared_ptr<TextureMetadata>>
    {
        public:
        void deinitialize();

        /// Mark id as binded for target
        void bind(GLenum target, size_t id);

        /// Track activation of texture unit
        void activate(size_t id);

        /// Get map of texture units
        TextureUnitTracker& getTextureUnits();
        /*
         * Helpers
         */
        /// Converts type (e.g. GL_TEXTURE_2D) to string
	static GLenum getParameterForType(GLenum type);

        /// Converts format (e.g. GL_RGBA & GL_UNSIGNED_BYTE) to sized format
        static GLenum convertToSizedFormat(GLenum internalFormat, GLenum size);

        /// Determines if given format is precise-size OpenGL texture format
        static bool isSizedFormat(GLenum format);
        private:
        TextureUnitTracker m_TextureUnits;
    };
} //namespace trackers
} //namespace ve
#endif
