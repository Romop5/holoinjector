#ifndef VE_IMGUI_ADAPTER_HPP
#define VE_IMGUI_ADAPTER_HPP
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
        private:
    };
} // namespace ve
#endif
