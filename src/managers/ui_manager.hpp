#ifndef VE_UI_MANAGER_HPP
#define VE_UI_MANAGER_HPP

#include <cstddef>
#include <memory>

namespace ve
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
