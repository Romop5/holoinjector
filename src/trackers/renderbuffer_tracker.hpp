#ifndef VE_RENDERBUFFER_TRACKER_HPP
#define VE_RENDERBUFFER_TRACKER_HPP

#include <memory>
#include "GL/gl.h"

#include "trackers/texture_tracker.hpp"

namespace ve
{
namespace trackers
{
    class RenderbufferMetadata: public TextureMetadata
    {
        public:
        RenderbufferMetadata(size_t id): TextureMetadata(id) {}
        // Override creation of shadow renderbuffer
        virtual void createShadowedTexture(size_t numOfLayers = 9) override;
    };
    class RenderbufferTracker: public ContextTracker<std::shared_ptr<RenderbufferMetadata>>
    {
    };
} //namespace trackers
} //namespace ve
#endif
