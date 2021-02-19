#ifndef VE_INSPECTOR_WIDGET_HPP
#define VE_INSPECTOR_WIDGET_HPP

namespace ve
{
    namespace trackers
    {
        class ShaderManager;
    }

    class InspectorWidget
    {
    public:
        InspectorWidget(trackers::ShaderManager& manager);
        void onDraw();
    private:
        trackers::ShaderManager& interface;
    };
}
#endif
