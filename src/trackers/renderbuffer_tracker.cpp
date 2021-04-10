/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        trackers/renderbuffer_tracker.cpp
*
*****************************************************************************/

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "logger.hpp"
#include "renderbuffer_tracker.hpp"
#include "utils/opengl_debug.hpp"

using namespace ve;
using namespace ve::trackers;
//-----------------------------------------------------------------------------
// RenderbufferMetadata
//-----------------------------------------------------------------------------
TextureType RenderbufferMetadata::getPhysicalTextureType()
{
    return TextureType::RENDERBUFFER;
}

void RenderbufferMetadata::createShadowedTexture(size_t numOfLayers)
{
    if (getWidth() == 0 || getHeight() == 0)
    {
        Logger::logError(" Failed to get texture size. Got ", getWidth(), "x", getHeight(), ENHANCER_POS);
        return;
    }
    Logger::logDebug("Creating shadow texture: renderbuffer: with resolution: ", getWidth(), "x",
        getHeight(), " and format: ", TextureMetadata::getFormatAsString(getFormat()), ENHANCER_POS);

    glGetError();
    GLuint layeredTexture;
    glGenTextures(1, &layeredTexture);
    ASSERT_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D_ARRAY, layeredTexture);
    ASSERT_GL_ERROR();

    auto format = getFormat();
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, getFormat(), getWidth(), getHeight(), numOfLayers);
    auto storageError = glGetError();
    if (storageError == GL_INVALID_ENUM)
    {
        Logger::logError("Failed to create renderbuffer with intercepted format. Falling back to GL_DEPTH_COMPONENT32");
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, getWidth(), getHeight(), numOfLayers);
        ASSERT_GL_ERROR();
    }
    else
    {
        Logger::logError("Error while creating texture for renderbuffer texture", ENHANCER_POS);
    }

    ASSERT_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    m_shadowedLayerVersionId = layeredTexture;
    assert(m_shadowedLayerVersionId != 0);

    // By definition, Renderbuffer is not suited for sampling
    // => no need to set up a texture view as renderbuffer is always coupled with FBO
    m_shadowTextureViewId = 0;
}
