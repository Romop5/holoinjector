#include <memory>
#include "GL/gl.h"

#include "utils/context_tracker.hpp"
#include "pipeline/program_metadata.hpp"

namespace ve
{
    class TextureMetadata
    {
        public:
        TextureMetadata(size_t id): m_Id(id) {}
        void setStorage(size_t width, size_t height, size_t levels, size_t layers, GLenum internalFormat);

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
        void createShadowedTexture(size_t numOfLayers = 9);
        private:
        size_t m_Id = 0;

        GLenum m_Type;
        GLenum m_Format;
        size_t m_Width = 0;
        size_t m_Height = 0;
        size_t m_Levels = 0;
        size_t m_Layers = 0;

        /*
         * Extras
         */
        size_t m_shadowedLayerVersionId = 0;
    };
    class TextureTracker: public ContextTracker<std::shared_ptr<TextureMetadata>>
    {
        public:
	static GLenum getParameterForType(GLenum type);
        private:
    };
}
