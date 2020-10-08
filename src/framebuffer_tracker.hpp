#ifndef VE_FRAMEBUFFER_TRACKER_HPP
#define VE_FRAMEBUFFER_TRACKER_HPP

#include <unordered_map>
#include <GL/gl.h>

namespace ve
{
    class FramebufferTracker
    {
        public:
            struct FramebufferMetadata
            {
                bool hasDepthBufferAttached = false;
                bool hasStencil = false;
                size_t numberOfColorBuffersAttached = 0;
            };

        void addFramebuffer(size_t id);
        bool hasFramebuffer(size_t id) const;
        void deleteFramebuffer(size_t id);

        void attach(GLenum attachment, GLuint texture);
        void attachDepth(size_t frameBuffer);
        void attachColor(size_t frameBuffer);
        void attachStencil(size_t frameBuffer);

        void bind(size_t id);
        size_t getCurrentFrameBuffer() const;

        // Heuristics
        bool isFBODefault() const;
        bool isFBOshadowMap() const;

        private:
        std::unordered_map<size_t, FramebufferMetadata> m_buffers;
        size_t m_currentFramebuffer = 0;
    };
} // namespace ve
#endif
