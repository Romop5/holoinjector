/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        ui/x11_sniffer.cpp
*
*****************************************************************************/

#include "ui/x11_sniffer.hpp"
#include "logger.hpp"

#include <X11/Xatom.h>

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
        /* X11 event system does not allow to report an empty event when user
         * calls blocking XNextEvent. Instead, we return an event which we
         * consider unharmful and which should keep status quo of underlying
         * application.
         * Originally, ClientMessage was used, but these caused collision with applications
         * such as NeHes.
         */
        auto msg = reinterpret_cast<XVisibilityEvent*>(event);
        msg->type = VisibilityNotify;
        msg->state = VisibilityUnobscured;
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
            Logger::logDebugPerFrame("XNextEvent KeyPress",
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

            Logger::logDebugPerFrame("MotionNotify", dx, dy);

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

void X11Sniffer::turnFullscreen()
{
    /*
    Atom window_type = XInternAtom(m_Display, "_NET_WM_WINDOW_TYPE", False);
    long value = XInternAtom(m_Display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(m_Display, m_Window, window_type,
    XA_ATOM, 32, PropModeReplace, (unsigned char *) &value,1 );
    */

    /*Atom wm_state   = XInternAtom (m_Display, "_NET_WM_STATE", true );
    Atom wm_fullscreen = XInternAtom (m_Display, "_NET_WM_STATE_FULLSCREEN", true );

    XChangeProperty(m_Display, m_Window, wm_state, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
    */

    //code is from [http://tonyobryan.com/index.php?article=9][1]
    typedef struct Hints
    {
        unsigned long   flags;
        unsigned long   functions;
        unsigned long   decorations;
        long            inputMode;
        unsigned long   status;
    } Hints;

    static bool shouldBeFullscreen = true;
    // See: https://metacpan.org/pod/X11::Protocol::WM#_MOTIF_WM_HINTS
    auto decorations = shouldBeFullscreen?0:1;
    shouldBeFullscreen = !shouldBeFullscreen;
    //code to remove decoration
    Hints hints;
    Atom property;
    hints.flags = 2;
    hints.decorations = decorations;

    property = XInternAtom(m_Display, "_MOTIF_WM_HINTS", true);
    XChangeProperty(m_Display, m_Window,property,property,32,PropModeReplace,(unsigned char *)&hints,5);
    XMapWindow(m_Display, m_Window);
}
