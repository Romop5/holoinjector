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
    class TextureMetadata
    {
        public:
        TextureMetadata(size_t id): m_Id(id) {}
        virtual ~TextureMetadata();
        void deinitialize();
        void setStorage(GLenum type, size_t width, size_t height, size_t levels, size_t layers, GLenum internalFormat);

        size_t getWidth() const;
        size_t getHeight() const;
        size_t getLayers() const;
        size_t getLevels() const;

        GLenum getType();
        GLenum getFormat();

        /*
         * Shadowing facility
         */
        bool hasShadowTexture() const;
        size_t getShadowedTextureId() const;
        size_t getTextureViewIdOfShadowedTexture() const;
        virtual void createShadowedTexture(size_t numOfLayers = 9);
        void setTextureViewToLayer(size_t layer);
        protected:
        size_t m_Id = 0;

        GLenum m_Type = 0;
        GLenum m_Format = 0;
        size_t m_Width = 0;
        size_t m_Height = 0;
        size_t m_Levels = 0;
        size_t m_Layers = 0;

        /*
         * Extras
         */
        size_t m_shadowedLayerVersionId = 0;
        size_t m_shadowTextureViewId = 0;
    };

    class TextureUnit: public ContextTracker<std::shared_ptr<TextureMetadata>>
    {
    };

    // Forwarding due to friend class
    class TextureTracker;

    class TextureUnitTracker: private BindableContextTracker<std::shared_ptr<TextureUnit>,false>
    {
        public:
        bool hasShadowedTextureBinded() const;
        void bindShadowedTexturesToLayer(size_t layer);
        protected:
        void activate(size_t id);
        void bind(size_t target, std::shared_ptr<TextureMetadata> texture);
        MapType& getUnits();
        friend class TextureTracker;
        private:
        using baseType = BindableContextTracker<std::shared_ptr<TextureUnit>,false>;
    };
    class TextureTracker: public ContextTracker<std::shared_ptr<TextureMetadata>>
    {
        public:
        void deinitialize();
	static GLenum getParameterForType(GLenum type);
        static GLenum convertToSizedFormat(GLenum internalFormat, GLenum size);
        static bool isSizedFormat(GLenum format);

        void bind(GLenum target, size_t id);
        void activate(size_t id);
        TextureUnitTracker& getTextureUnits();

        private:
        TextureUnitTracker m_TextureUnits;
    };
} //namespace trackers
} //namespace ve
#endif
