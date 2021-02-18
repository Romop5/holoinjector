#ifndef VE_UI_MANAGER_HPP
#define VE_UI_MANAGER_HPP

#include <cstddef>

namespace ve
{
    class Context;

namespace managers
{
    class UIManager
    {
        public:
            void initialize(Context& context);
            void onKeyPressed(Context& context, size_t key);
    };
}
}
#endif
