/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        ui/inspector_widget.hpp
*
*****************************************************************************/

#ifndef HI_INSPECTOR_WIDGET_HPP
#define HI_INSPECTOR_WIDGET_HPP

namespace hi
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
