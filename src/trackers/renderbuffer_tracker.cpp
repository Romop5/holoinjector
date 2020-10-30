#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

#include "renderbuffer_tracker.hpp"

using namespace ve;
//-----------------------------------------------------------------------------
// RenderbufferMetadata
//-----------------------------------------------------------------------------
void RenderbufferMetadata::createShadowedTexture(size_t numOfLayers)
{
    GLuint textures[2];
    glGenTextures(1, textures);

    glBindTexture(GL_TEXTURE_2D_ARRAY, textures[0]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, getLevels(), getFormat(), getWidth(),getHeight(), numOfLayers);
    m_shadowedLayerVersionId = textures[0];

    // By definition, Renderbuffer is not suited for sampling
    // => no need to set up a texture view as renderbuffer is always coupled with FBO
    m_shadowTextureViewId = 0;
}

