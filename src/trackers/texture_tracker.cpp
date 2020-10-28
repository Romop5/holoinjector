#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

#include "texture_tracker.hpp"
#include <cassert>

using namespace ve;

void TextureMetadata::setStorage(size_t width, size_t height, size_t levels, size_t layers, GLenum internalFormat)
{
    m_Width = width;
    m_Height = height;

    m_Levels = levels;
    m_Layers = layers;
    m_Format = internalFormat;
}

size_t TextureMetadata::getWidth() const
{
    return m_Width;
}
size_t TextureMetadata::getHeight() const
{
    return m_Height;
}
size_t TextureMetadata::getLayers() const
{
    return m_Layers;
}
size_t TextureMetadata::getLevels() const
{
    return m_Levels;
}

GLenum TextureMetadata::getType()
{
    return m_Type;
}
GLenum TextureMetadata::getFormat()
{
    return m_Format;
}


bool TextureMetadata::hasShadowTexture() const
{
    return m_shadowedLayerVersionId != 0;
}
size_t TextureMetadata::getShadowedTextureId() const
{
    return m_shadowedLayerVersionId;
}
void TextureMetadata::createShadowedTexture(size_t numOfLayers)
{
    GLuint texture;
    glGenTextures(1, &texture);
    assert(getType() == GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, getLevels(), getFormat(), getWidth(),getHeight(), numOfLayers);
    m_shadowedLayerVersionId = texture;
}

//-----------------------------------------------------------------------------
// Texture Tracker
//-----------------------------------------------------------------------------

GLenum TextureTracker::getParameterForType(GLenum type)
{
    switch(type)
    {
        case GL_TEXTURE_1D: return GL_TEXTURE_BINDING_1D;
        case GL_TEXTURE_1D_ARRAY: return GL_TEXTURE_BINDING_1D_ARRAY;
        case GL_TEXTURE_2D: return GL_TEXTURE_BINDING_2D;
        case GL_TEXTURE_2D_ARRAY: return GL_TEXTURE_BINDING_2D_ARRAY;
        case GL_TEXTURE_2D_MULTISAMPLE: return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
        case GL_TEXTURE_3D: return GL_TEXTURE_BINDING_3D;
        default:
            assert(false);
    }
    return 0;
}
