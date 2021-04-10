/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        trackers/renderbuffer_tracker.hpp
*
*****************************************************************************/

#ifndef VE_RENDERBUFFER_TRACKER_HPP
#define VE_RENDERBUFFER_TRACKER_HPP

#include "GL/gl.h"
#include <memory>

#include "trackers/texture_tracker.hpp"

namespace ve
{
namespace trackers
{
    class RenderbufferMetadata : public TextureMetadata
    {
    public:
        RenderbufferMetadata(size_t id)
            : TextureMetadata(id)
        {
        }
        virtual TextureType getPhysicalTextureType() override;
        // Override creation of shadow renderbuffer
        virtual void createShadowedTexture(size_t numOfLayers = 9) override;
    };
    class RenderbufferTracker : public ContextTracker<std::shared_ptr<RenderbufferMetadata>>
    {
    };
} //namespace trackers
} //namespace ve
#endif
