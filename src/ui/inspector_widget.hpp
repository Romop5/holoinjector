#ifndef VE_INSPECTOR_WIDGET_HPP
#define VE_INSPECTOR_WIDGET_HPP

namespace ve
{
    namespace trackers
    {
        class ShaderTracker;
        class FramebufferTracker;
        class TextureTracker;
    }

    class InspectorWidget
    {
    public:
        InspectorWidget(trackers::ShaderTracker& shaderTracker, trackers::FramebufferTracker& fboTracker, trackers::TextureTracker& textureTracker);
        void onDraw();
    private:
        trackers::ShaderTracker& shaderInterface;
        trackers::FramebufferTracker& interfaceFBO;
        trackers::TextureTracker& interfaceTextureTracker;
    };
}
#endif
