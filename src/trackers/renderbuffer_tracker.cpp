#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "renderbuffer_tracker.hpp"
#include "logger.hpp"

using namespace ve;
using namespace ve::trackers;
//-----------------------------------------------------------------------------
// RenderbufferMetadata
//-----------------------------------------------------------------------------
void RenderbufferMetadata::createShadowedTexture(size_t numOfLayers)
{
    if(getWidth() == 0 || getHeight() == 0)
    {
        Logger::logError("[Repeater]: Failed to get texture size. Got ",getWidth(),"x",getHeight(), ENHANCER_POS);
        return;
    }
    glGetError();
    GLuint textures[2];
    glGenTextures(1, textures);

    glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);
    assert(getFormat() != GL_ZERO);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, getFormat(), getWidth(), getHeight(), numOfLayers);
    assert(glGetError() == GL_NO_ERROR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    m_shadowedLayerVersionId = textures[0];

    // By definition, Renderbuffer is not suited for sampling
    // => no need to set up a texture view as renderbuffer is always coupled with FBO
    m_shadowTextureViewId = 0;
}

