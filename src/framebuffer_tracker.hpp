#ifndef VE_FRAMEBUFFER_TRACKER_HPP
#define VE_FRAMEBUFFER_TRACKER_HPP

#include <unordered_map>
#include <memory>

#include <GL/gl.h>
#include "utils/context_tracker.hpp"

namespace ve
{
    struct FramebufferMetadata
    {
        bool hasDepthBufferAttached = false;
        bool hasStencil = false;
        size_t numberOfColorBuffersAttached = 0;
    };

    class FramebufferTracker: public BindableContextTracker<std::shared_ptr<FramebufferMetadata>>
    {
        public:
        void attach(GLenum attachment, GLuint texture);
        void attachDepth(size_t frameBuffer);
        void attachColor(size_t frameBuffer);
        void attachStencil(size_t frameBuffer);

        // Heuristics
        bool isFBODefault() const;
        bool isFBOshadowMap() const;
    };
} // namespace ve
#endif
