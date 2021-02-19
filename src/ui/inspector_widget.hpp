#ifndef VE_INSPECTOR_WIDGET_HPP
#define VE_INSPECTOR_WIDGET_HPP

namespace ve
{
    namespace trackers
    {
        class ShaderTracker;
    }

    class InspectorWidget
    {
    public:
        InspectorWidget(trackers::ShaderTracker& manager);
        void onDraw();
    private:
        trackers::ShaderTracker& interface;
    };
}
#endif
