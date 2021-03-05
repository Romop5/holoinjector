#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "texture_tracker.hpp"
#include <cassert>
#include "logger.hpp"
#include "utils/opengl_debug.hpp"

using namespace ve;
using namespace ve::trackers;

namespace helper
{
    template <typename T>
    inline T max(T a, T b)
    {
        return (a > b)?a:b;
    }
}

//-----------------------------------------------------------------------------
// TextureMetadata
//-----------------------------------------------------------------------------
TextureMetadata::~TextureMetadata()
{
    deinitialize();
}

void TextureMetadata::deinitialize()
{
    GLuint textures[2];
    size_t numOfTextures = 0;
    if(m_shadowedLayerVersionId)
    {
        textures[numOfTextures++] = m_shadowedLayerVersionId;
        m_shadowedLayerVersionId = 0;
    }
    if(m_shadowTextureViewId)
    {
        textures[numOfTextures++] = m_shadowTextureViewId;
        m_shadowTextureViewId = 0;
    }
    glDeleteTextures(numOfTextures,textures);
}

void TextureMetadata::setStorage(GLenum type, size_t width, size_t height, size_t levels, size_t layers, GLenum internalFormat)
{
    m_Type = type;
    m_Format = internalFormat;
    // Hack: ignore mipmaps
    if(levels > 0)
    {
        return;
    }
    m_Width = width;
    m_Height = height;

    m_Levels = (levels == 0)?1:levels;
    m_Layers = layers;
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
    if(getType() != GL_TEXTURE_2D)
    {
        Logger::logError("[Repeater]: Expected GL_TEXTURE_2D as texture type of FBO's attachment, got ",getTypeAsString(getType()), " instead",ENHANCER_POS);
        CLEAR_GL_ERROR();
        return;
    }

    if(getWidth() == 0 || getHeight() == 0)
    {
        Logger::logError("[Repeater]: Failed to get texture size. Got ",getWidth(),"x",getHeight(), ENHANCER_POS);
        return;
    }

    CLEAR_GL_ERROR();
    GLuint textures[2];
    glGenTextures(1, textures);
    ASSERT_GL_ERROR();
    ASSERT_GL_EQ(getType(),GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);
    ASSERT_GL_ERROR();
    assert(getLevels() == 1);
    assert(getFormat() != GL_ZERO);

    assert(getWidth() > 0);
    assert(getHeight() > 0);
    const auto shadowTextureWidth = getWidth();
    const auto shadowTextureHeight = getHeight();
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, getFormat(), shadowTextureWidth,  shadowTextureHeight, numOfLayers);
    if(glGetError() != GL_NO_ERROR)
    {
        Logger::logError("[Repeater] Failed to set layered shadow texture's storage", ENHANCER_POS);
        return;
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_shadowedLayerVersionId = textures[0];

    // set texture view as original texture
    setTextureViewToLayer(0);
}

void TextureMetadata::setTextureViewToLayer(size_t layer)
{
    GLuint viewId = m_shadowTextureViewId;
    assert(m_shadowedLayerVersionId != 0);
    // If view already exists, delete it
    if(m_shadowTextureViewId)
        glDeleteTextures(1, &viewId);
    glGenTextures(1, &viewId);
    glTextureView(viewId, GL_TEXTURE_2D, m_shadowedLayerVersionId, getFormat(), 0, getLevels(), layer, 1);
    if(glGetError() != GL_NO_ERROR)
    {
        Logger::logError("[Repeater]: Failed to create texture view.", ENHANCER_POS);
        return;
    }
    m_shadowTextureViewId = viewId;
}

std::string TextureMetadata::getTypeAsString(GLenum type)
{
    switch(type)
    {
        case GL_RENDERBUFFER: return "GL_RENDERBUFFER";
        case GL_TEXTURE_1D: return "GL_TEXTURE_1D";
        case GL_TEXTURE_1D_ARRAY: return "GL_TEXTURE_1D_ARRAY";
        case GL_TEXTURE_2D: return "GL_TEXTURE_2D";
        case GL_PROXY_TEXTURE_2D: return "GL_PROXY_TEXTURE_2D";
        case GL_TEXTURE_2D_ARRAY: return "GL_TEXTURE_2D_ARRAY";
        case GL_TEXTURE_2D_MULTISAMPLE: return "GL_TEXTURE_2D_MULTISAMPLE";
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: return "GL_TEXTURE_2D_MULTISAMPLE_ARRAY";
        case GL_TEXTURE_3D: return "GL_TEXTURE_3D";

        case GL_TEXTURE_CUBE_MAP: return "GL_TEXTURE_CUBE_MAP";
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X: return "GL_TEXTURE_CUBE_MAP_POSITIVE_X";
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: return "GL_TEXTURE_CUBE_MAP_POSITIVE_Y";
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: return "GL_TEXTURE_CUBE_MAP_POSITIVE_Z";
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: return "GL_TEXTURE_CUBE_MAP_POSITIVE_X";
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: return "GL_TEXTURE_CUBE_MAP_POSITIVE_Y";
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: return "GL_TEXTURE_CUBE_MAP_POSITIVE_Z";
        default:
        return "UKNOWN_TYPE_"+std::to_string(type);
    }
}


std::string TextureMetadata::getFormatAsString(GLenum type)
{
    switch(type)
    {
        case GL_DEPTH_COMPONENT16: return "GL_DEPTH_COMPONENT16";
        case GL_DEPTH_COMPONENT32: return "GL_DEPTH_COMPONENT32";
        case GL_DEPTH_COMPONENT32F: return "GL_DEPTH_COMPONENT32F";
        case GL_DEPTH24_STENCIL8: return "GL_DEPTH24_STENCIL8";
        case GL_R11F_G11F_B10F: return "GL_R11F_G11F_B10F";
        case GL_UNSIGNED_INT_10F_11F_11F_REV: return "GL_UNSIGNED_INT_10F_11F_11F_REV";
        case GL_RGBA16F: return "GL_RGBA16F";
        case GL_R16: return "GL_R16";
        case GL_R16F: return "GL_R16F";
        case GL_R32F: return "GL_R32F";
        case GL_R8: return "GL_R8";
        case GL_RGB16: return "GL_RGB16";
        case GL_RGB32F: return "GL_RGB32F";
        case GL_RGB8: return "GL_RGB8";
        case GL_RGBA16: return "GL_RGBA16";
        case GL_RGBA32F: return "GL_RGBA32F";
        case GL_RGBA8: return "GL_RGBA8";
        default:
            return "UNKNOWN_FORMAT_"+std::to_string(type);
    }
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
        assert(id <= 80);
        for(auto& [target, texture]: unit->getMap())
        {
            if(!texture->hasShadowTexture())
                continue;
            texture->setTextureViewToLayer(layer);
            glActiveTexture(GL_TEXTURE0+id);
            auto textureView = texture->getTextureViewIdOfShadowedTexture();
            if(textureView)
                glBindTexture(target, textureView);
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
void TextureTracker::deinitialize()
{
    for(auto& [id,texture]: getMap())
    {
        texture->deinitialize();
    }
}

GLenum TextureTracker::getParameterForType(GLenum type)
{
    switch(type)
    {
        case GL_TEXTURE_1D: return GL_TEXTURE_BINDING_1D;
        case GL_TEXTURE_1D_ARRAY: return GL_TEXTURE_BINDING_1D_ARRAY;
        case GL_TEXTURE_2D: 
        case GL_PROXY_TEXTURE_2D: 
            return GL_TEXTURE_BINDING_2D;
        case GL_TEXTURE_2D_ARRAY: return GL_TEXTURE_BINDING_2D_ARRAY;
        case GL_TEXTURE_2D_MULTISAMPLE: return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
        case GL_TEXTURE_3D: return GL_TEXTURE_BINDING_3D;

        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                return GL_TEXTURE_BINDING_CUBE_MAP;
        default:
            assert(false);
    }
    return 0;
}


GLenum TextureTracker::convertToSizedFormat(GLenum internalFormat, GLenum size)
{
    switch(internalFormat)
    {
        case GL_RED:
            switch(size)
            {
                case GL_BYTE: return GL_R8;
                case GL_UNSIGNED_BYTE: return GL_R8;
                case GL_SHORT: return GL_R16;
                case GL_UNSIGNED_SHORT: return GL_R16;
                case GL_INT: return GL_R32F;
                case GL_UNSIGNED_INT: return GL_R32F;
                case GL_FLOAT: return GL_R32F;
                default:
                   return 0;
            }

        case GL_BGR: [[fallthrough]];
        case GL_RGB:
            switch(size)
            {
                case GL_BYTE: return GL_RGB8;
                case GL_UNSIGNED_BYTE: return GL_RGB8;
                case GL_SHORT: return GL_RGB16;
                case GL_UNSIGNED_SHORT: return GL_RGB16;
                case GL_INT: return GL_RGB32F;
                case GL_UNSIGNED_INT: return GL_RGB32F;
                case GL_FLOAT: return GL_RGB32F;
                default:
                   return 0;
            }
        case GL_BGRA: [[fallthrough]];
        case GL_RGBA:
        {
            switch(size)
            {
                case GL_BYTE: return GL_RGBA8;
                case GL_UNSIGNED_BYTE: return GL_RGBA8;
                case GL_SHORT: return GL_RGBA16;
                case GL_UNSIGNED_SHORT: return GL_RGBA16;
                case GL_INT: return GL_RGBA32F;
                case GL_UNSIGNED_INT: return GL_RGBA32F;
                case GL_FLOAT: return GL_RGBA32F;
                default:
                   return 0;
            }
        }
        case GL_DEPTH_COMPONENT:
            return GL_DEPTH_COMPONENT32F;
        default:
            return 0;
    }
    return 0;
}

bool TextureTracker::isSizedFormat(GLenum format)
{
    switch(format)
    {
        case GL_BGRA:
        case GL_BGR:
        case GL_RGB:
        case GL_RGBA:
        case GL_RED:
            return true;
    }
    // TODO: add all cases
    return false;
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
