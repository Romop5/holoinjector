#ifndef VE_INSPECTOR_WIDGET_HPP
#define VE_INSPECTOR_WIDGET_HPP

namespace ve
{
    namespace trackers
    {
        class ShaderTracker;
        class FramebufferTracker;
        class TextureTracker;
        class RenderbufferTracker;
    }

    class InspectorWidget
    {
    public:
        InspectorWidget(trackers::ShaderTracker& shaderTracker, trackers::FramebufferTracker& fboTracker, trackers::TextureTracker& textureTracker, trackers::RenderbufferTracker& renderbufferTracker);
        void onDraw();
    private:
        trackers::ShaderTracker& shaderInterface;
        trackers::FramebufferTracker& interfaceFBO;
        trackers::TextureTracker& interfaceTextureTracker;
        trackers::RenderbufferTracker& interfaceRenderbufferTracker;
    };
}
#endif
