/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        managers/ui_manager.hpp
*
*****************************************************************************/

#ifndef HI_UI_MANAGER_HPP
#define HI_UI_MANAGER_HPP

#include <cstddef>
#include <memory>

namespace hi
{
class Context;

class InspectorWidget;
namespace managers
{
    class UIManager
    {
    public:
        UIManager();
        ~UIManager();
        void initialize(Context& context);
        void deinitialize(Context& context);
        void onKeyPressed(Context& context, size_t key);
        void onDraw(Context& context);

    private:
        void registerCallbacks(Context& context);
        std::unique_ptr<InspectorWidget> inspectorWidget;
    };
}
}
#endif
