#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

#include "texture_tracker.hpp"
#include <cassert>

using namespace ve;

//-----------------------------------------------------------------------------
// TextureMetadata
//-----------------------------------------------------------------------------
TextureMetadata::~TextureMetadata()
{
    GLuint textures[2];
    size_t numOfTextures = 0;
    if(m_shadowedLayerVersionId)
        textures[numOfTextures++] = m_shadowedLayerVersionId;
    if(m_shadowTextureViewId)
        textures[numOfTextures++] = m_shadowTextureViewId;
    glDeleteTextures(numOfTextures,textures);
}

void TextureMetadata::setStorage(GLenum type, size_t width, size_t height, size_t levels, size_t layers, GLenum internalFormat)
{
    m_Width = width;
    m_Height = height;

    m_Levels = (levels == 0)?1:levels;
    m_Layers = layers;
    m_Format = internalFormat;
    m_Type = type;
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

size_t TextureMetadata::getTextureViewIdOfShadowedTexture() const
{
    return m_shadowTextureViewId;
}
void TextureMetadata::createShadowedTexture(size_t numOfLayers)
{
    GLuint textures[2];
    glGenTextures(1, textures);
    assert(getType() == GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, getLevels(), getFormat(), getWidth(),getHeight(), numOfLayers);
    m_shadowedLayerVersionId = textures[0];

    // set texture view as original texture
    // use layer 0 as default
    glGenTextures(1, &textures[1]);
    glTextureView(textures[1], GL_TEXTURE_2D, m_shadowedLayerVersionId, getFormat(), 0, getLevels(), 0, 1);
    m_shadowTextureViewId = textures[1];
}

void TextureMetadata::setTextureViewToLayer(size_t layer)
{
    GLuint viewId = m_shadowTextureViewId;
    assert(m_shadowedLayerVersionId != 0);
    if(m_shadowedLayerVersionId != 0)
        glDeleteTextures(1, &viewId);
    glGenTextures(1, &viewId);
    glTextureView(viewId, GL_TEXTURE_2D, m_shadowedLayerVersionId, getFormat(), 0, getLevels(), layer, 1);
    m_shadowedLayerVersionId = viewId;
}

//-----------------------------------------------------------------------------
// TextureUnitTracker
//-----------------------------------------------------------------------------
bool TextureUnitTracker::hasShadowedTextureBinded() const
{
    for(auto& [id, unit]: getConstMap())
    {
        for(auto& [target, texture]: unit->getConstMap())
        {
            if(texture->hasShadowTexture())
                return true;
        }
    }
    return false;
}

void TextureUnitTracker::bindShadowedTexturesToLayer(size_t layer)
{
    GLint id;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &id);
    for(auto& [id, unit]: getMap())
    {
        for(auto& [target, texture]: unit->getMap())
        {
            if(!texture->hasShadowTexture())
                continue;
            texture->setTextureViewToLayer(layer);
            //glBindTextureUnit(id, texture->getTextureViewIdOfShadowedTexture());
            glActiveTexture(id);
            glBindTexture(target, texture->getTextureViewIdOfShadowedTexture());
        }
    }
    glActiveTexture(id);
}

void TextureUnitTracker::activate(size_t id)
{
    baseType::bind(id);
}

void TextureUnitTracker::bind(size_t target, std::shared_ptr<TextureMetadata> texture)
{
    if(texture == nullptr)
    {
        if(!has(getBoundId()))
            return;
        get(getBoundId())->remove(target);
    } else {
        if(!has(getBoundId()))
            add(getBoundId(), std::make_shared<TextureUnit>());
        getBound()->add(target, texture);
    }
}
TextureUnitTracker::MapType& TextureUnitTracker::getUnits()
{
    return getMap();
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


GLenum TextureTracker::convertToSizedFormat(GLenum internalFormat, GLenum size)
{
    switch(internalFormat)
    {
        case GL_RGB:
            switch(size)
            {
                case GL_UNSIGNED_BYTE: return GL_RGB8;
                case GL_UNSIGNED_INT: return GL_RGB32I;
                case GL_FLOAT: return GL_RGB32F;
                default:
                   return 0;
            }
        case GL_RGBA:
        {
            switch(size)
            {
                case GL_UNSIGNED_BYTE: return GL_RGBA8;
                case GL_UNSIGNED_INT: return GL_RGBA32I;
                case GL_FLOAT: return GL_RGBA32F;
                default:
                   return 0;
            }
        }
        case GL_DEPTH_COMPONENT:
            return size;
        default:
            return 0;
    }
    return 0;
}

void TextureTracker::bind(GLenum target, size_t id)
{
    if(id == 0)
    {
        m_TextureUnits.bind(target,nullptr);
        return;
    }
    assert(has(id));
    m_TextureUnits.bind(target,get(id));
}

void TextureTracker::activate(size_t id)
{
    getTextureUnits().activate(id);
}

TextureUnitTracker& TextureTracker::getTextureUnits()
{
    return m_TextureUnits;
}
