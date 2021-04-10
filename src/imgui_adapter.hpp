/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        imgui_adapter.hpp
*
*****************************************************************************/

#ifndef VE_IMGUI_ADAPTER_HPP
#define VE_IMGUI_ADAPTER_HPP

#include <cstddef>

namespace ve
{
    // Fwd
    class Context;

    /**
     * @brief Adapts DearImgui into project
     */
    class ImguiAdapter
    {
        public:
            /// Prepare resources
            bool initialize();
            /// Start accepting user's definitions
            void beginFrame(Context& context);
            /// End of user's definitions and prepare draw buffer
            void endFrame();
            /// Render draw buffer
            void renderCurrentFrame();
            /// Clean up
            void destroy();

            void onKey(size_t key, bool isDown = true);
            void onMousePosition(float x, float y);
            void onButton(size_t buttonID, bool isPressed);

            bool isVisible();
            void setVisibility(bool isVisible);
            void setScaling(float scale);
        private:
            bool m_Visibility = false;
            float m_Scaling = 1.0;

            float posX = 0.0;
            float posY = 0.0;
    };
} // namespace ve
#endif
