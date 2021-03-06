/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        imgui_adapter.cpp
*
*****************************************************************************/

#include "imgui_adapter.hpp"

#include <GL/glew.h>
#include <X11/keysym.h>
#include <cctype>
#include <imgui_impl_opengl3.h>

#include "context.hpp"
#include "pipeline/viewport_area.hpp"

using namespace hi;

namespace helper
{
size_t mapXKeyTo256Array(size_t xkey)
{
    // Map ASCII to ASCII A-Z (already done by X11 lib)
    if (XK_space <= xkey && XK_asciitilde >= xkey)
        return xkey;
    switch (xkey)
    {
    case XK_Tab:
        return 0;
    case XK_Left:
        return 1;
    case XK_Right:
        return 2;
    case XK_Up:
        return 4;
    case XK_Down:
        return 5;
    case XK_Page_Up:
        return 6;
    case XK_Page_Down:
        return 7;
    case XK_Home:
        return 8;
    case XK_End:
        return 9;
    case XK_Insert:
        return 10;
    case XK_Delete:
        return 11;
    case XK_space:
        return 12;
    case XK_Return:
        return 13;
    case XK_Escape:
        return 14;
    }
    return 255;
}
};

bool hi::ImguiAdapter::initialize()
{
    glewInit();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.WantCaptureMouse = true;
    io.WantCaptureKeyboard = true;
    //io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

    io.KeyMap[ImGuiKey_Tab] = helper::mapXKeyTo256Array(XK_Tab);
    io.KeyMap[ImGuiKey_LeftArrow] = helper::mapXKeyTo256Array(XK_Left);
    io.KeyMap[ImGuiKey_RightArrow] = helper::mapXKeyTo256Array(XK_Right);
    io.KeyMap[ImGuiKey_UpArrow] = helper::mapXKeyTo256Array(XK_Up);
    io.KeyMap[ImGuiKey_DownArrow] = helper::mapXKeyTo256Array(XK_Down);
    io.KeyMap[ImGuiKey_PageUp] = helper::mapXKeyTo256Array(XK_Page_Up);
    io.KeyMap[ImGuiKey_PageDown] = helper::mapXKeyTo256Array(XK_Page_Down);
    io.KeyMap[ImGuiKey_Home] = helper::mapXKeyTo256Array(XK_Home);
    io.KeyMap[ImGuiKey_End] = helper::mapXKeyTo256Array(XK_End);
    io.KeyMap[ImGuiKey_Insert] = helper::mapXKeyTo256Array(XK_Insert);
    io.KeyMap[ImGuiKey_Delete] = helper::mapXKeyTo256Array(XK_Delete);
    io.KeyMap[ImGuiKey_Backspace] = helper::mapXKeyTo256Array(XK_BackSpace);
    io.KeyMap[ImGuiKey_Space] = helper::mapXKeyTo256Array(XK_space);
    io.KeyMap[ImGuiKey_Enter] = helper::mapXKeyTo256Array(XK_Return);
    io.KeyMap[ImGuiKey_Escape] = helper::mapXKeyTo256Array(XK_Escape);
    io.KeyMap[ImGuiKey_KeyPadEnter] = helper::mapXKeyTo256Array(XK_Return);
    io.KeyMap[ImGuiKey_A] = XK_A;
    io.KeyMap[ImGuiKey_C] = XK_C;
    io.KeyMap[ImGuiKey_V] = XK_V;
    io.KeyMap[ImGuiKey_X] = XK_X;
    io.KeyMap[ImGuiKey_Y] = XK_Y;
    io.KeyMap[ImGuiKey_Z] = XK_Z;

    io.MouseDrawCursor = true;
    ImGui_ImplOpenGL3_Init();
    return true;
}

void hi::ImguiAdapter::beginFrame(Context& context)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    w = context.getCurrentViewport().getWidth();
    h = context.getCurrentViewport().getHeight();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
    io.FontGlobalScale = m_Scaling;

    // Setup time step
    io.DeltaTime = (float)(1.0f / 60.0f);

    ImGui::NewFrame();
}
void hi::ImguiAdapter::endFrame()
{
    ImGui::Render();
}

void hi::ImguiAdapter::renderCurrentFrame()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void hi::ImguiAdapter::destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
}

void hi::ImguiAdapter::onKey(size_t key, bool isDown)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[helper::mapXKeyTo256Array(key)] = isDown;
    if (isDown && std::isalnum(key))
    {
        io.AddInputCharacter(key);
    }
}

void hi::ImguiAdapter::onMousePosition(float x, float y)
{
    posX += x;
    posY += y;
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(posX, posY);
}

void hi::ImguiAdapter::onButton(size_t buttonID, bool isPressed)
{
    assert(buttonID < 5);
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(posX, posY);
    if (buttonID == 3)
    {
        io.MouseWheel = 1.0;
    }
    else if (buttonID == 4)
    {
        io.MouseWheel = -1.0;
    }
    else
    {
        io.MouseDown[buttonID] = isPressed;
    }
}

bool hi::ImguiAdapter::isVisible()
{
    return m_Visibility;
}
void hi::ImguiAdapter::setVisibility(bool isVisible)
{
    m_Visibility = isVisible;
}

void hi::ImguiAdapter::setScaling(float scale)
{
    m_Scaling = scale;
}
