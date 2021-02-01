#ifndef VE_UI_X11_SNIFFER_HPP
#define VE_UI_X11_SNIFFER_HPP

#include <functional>
#include <X11/Xlib.h>

namespace ve
{
    class X11Sniffer
    {
        public:
        using KeyEventHandler = std::function<bool(size_t key, bool isDown)>;
        using MouseMoveHandler = std::function<bool(float x, float y)>;
        using ButtonEventHandler = std::function<bool(size_t buttonID, bool isPressed)>;

        void registerOnKeyCallback(KeyEventHandler);
        void registerOnMouseMoveCallback(MouseMoveHandler);
        void registerOnButtonCallback(ButtonEventHandler);

        /* Event */
        /// Process keyboard/mouse events
        int onXNextEvent(Display *display, XEvent* event);
        int onXWarpPointer(Display* display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y);

        Window&     getWindow();
        Display*    getDisplay();
        private:
        KeyEventHandler m_KeyEventCallback;
        MouseMoveHandler m_MouseEventCallback;
        ButtonEventHandler m_ButtonEventCallback;

        struct {
            int m_LastXPosition = 0;
            int m_LastYPosition = 0;
        } X11MouseHook;

        Window   m_Window;
        Display* m_Display = nullptr;

    };
} // namespace ve
#endif
