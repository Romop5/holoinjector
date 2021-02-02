#include "ui/x11_sniffer.hpp"
//#include <X11/keysym.h>
#include "logger.hpp"

using namespace ve;

void ve::X11Sniffer::registerOnKeyCallback(KeyEventHandler handler)
{
    m_KeyEventCallback = std::move(handler);
}

void ve::X11Sniffer::registerOnMouseMoveCallback(MouseMoveHandler handler)
{
    m_MouseEventCallback = std::move(handler);
}

void ve::X11Sniffer::registerOnButtonCallback(ButtonEventHandler handler)
{
    m_ButtonEventCallback = std::move(handler);
}

int ve::X11Sniffer::onXNextEvent(Display *display, XEvent* event_return)
{
    auto anyEvent = reinterpret_cast<XAnyEvent*>(event_return);

    auto fillEmptyEvent = [](XEvent* event)
    {
        auto msg = reinterpret_cast<XClientMessageEvent*>(event);
        msg->type = ClientMessage;
        msg->message_type = None;
        return;
    };
    auto returnVal = XNextEvent(display,event_return);

    if(event_return->type == KeyPress || event_return->type == KeyRelease)
    {
        auto keyEvent = reinterpret_cast<XKeyEvent*>(event_return);
        static unsigned long lastSerial = 0;
        if(keyEvent->serial != lastSerial)
        {
            lastSerial = keyEvent->serial;
            Logger::log("[Repeater] XNextEvent KeyPress {}, {} - {} - {} - [{} {}] [{} {}] {}\n",
                    keyEvent->type,keyEvent->serial, keyEvent->send_event,
                    static_cast<void*>(keyEvent->display),
                    keyEvent->x,keyEvent->y,keyEvent->state,keyEvent->keycode, keyEvent->same_screen);
            auto keySym = XLookupKeysym(reinterpret_cast<XKeyEvent*>(event_return), 0);

            m_Window = keyEvent->window;
            m_Display = keyEvent->display;
            if(m_KeyEventCallback(keySym,(event_return->type == KeyPress)))
            {
                fillEmptyEvent(event_return);
            }
        }
    }

    if(event_return->type == MotionNotify)
    {
        auto mouseEvent = reinterpret_cast<XMotionEvent*>(event_return);
        auto dx = mouseEvent->x-X11MouseHook.m_LastXPosition;
        auto dy = mouseEvent->y-X11MouseHook.m_LastYPosition;
        if(m_MouseEventCallback(dx,dy))
        {
            std::swap(X11MouseHook.m_LastXPosition,mouseEvent->x);
            std::swap(X11MouseHook.m_LastYPosition,mouseEvent->y);

            Logger::log("[Repeater] {} {}\n", dx, dy);

            fillEmptyEvent(event_return);
        }
    }

    if(event_return->type == ButtonPress || event_return->type == ButtonRelease)
    {
        const auto isPressed = (event_return->type == ButtonPress);
        auto event = reinterpret_cast<XButtonPressedEvent*>(event_return);

        if(m_ButtonEventCallback(event->button-1, isPressed))
        {
            fillEmptyEvent(event_return);
        }
    }

    return returnVal;
}

int ve::X11Sniffer::onXWarpPointer(Display* display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y)
{
    return 0;
}

Window& X11Sniffer::getWindow()
{
    return m_Window;
}

Display* X11Sniffer::getDisplay()
{
    return m_Display;
}
