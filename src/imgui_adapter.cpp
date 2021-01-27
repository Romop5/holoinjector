#include "imgui_adapter.hpp"
#include <imgui_impl_opengl3.h>
#include "context.hpp"

#include <X11/keysym.h>
using namespace ve;

namespace helper
{
    size_t mapXKeyTo256Array(size_t xkey)
    {
        // Map ASCII to ASCII A-Z (already done by X11 lib)
        if(XK_space<= xkey && XK_asciitilde >= xkey)
            return xkey;
        switch(xkey)
        {
            case XK_Tab: return 0;
            case XK_Left: return 1;
            case XK_Right: return 2;
            case XK_Up: return 4;
            case XK_Down: return 5;
            case XK_Page_Up: return 6;
            case XK_Page_Down: return 7;
            case XK_Home: return 8;
            case XK_End: return 9;
            case XK_Insert: return 10;
            case XK_Delete: return 11;
            case XK_space: return 12;
            case XK_Return: return 13;
            case XK_Escape: return 14;
        }
        return 255;
    }
};

bool ve::ImguiAdapter::initialize()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.WantCaptureMouse = true;
    io.WantCaptureKeyboard= true;
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

void ve::ImguiAdapter::beginFrame(Context& context)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    w = context.currentViewport.getWidth();
    h = context.currentViewport.getHeight();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(1.0,1.0);

    // Setup time step
    io.DeltaTime = (float)(1.0f / 60.0f);

    ImGui::NewFrame();
}
void ve::ImguiAdapter::endFrame()
{
    ImGui::Render();
}

void ve::ImguiAdapter::renderCurrentFrame()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ve::ImguiAdapter::destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
}

void ve::ImguiAdapter::onKey(size_t key, bool isDown)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[helper::mapXKeyTo256Array(key)] = isDown;
    if(isDown && std::isalnum(key))
    {
        io.AddInputCharacter(key);
    }
}

void ve::ImguiAdapter::onMousePosition(float x, float y)
{
    static float posX = 0.0;
    static float posY = 0.0;

    posX += x;
    posY += y;
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(posX,posY);
}

void ve::ImguiAdapter::onButton(size_t buttonID, bool isPressed)
{
    assert(buttonID < 5);
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[buttonID] = isPressed;
}


bool ve::ImguiAdapter::isVisible()
{
    return m_Visibility;
}
void ve::ImguiAdapter::setVisibility(bool isVisible)
{
    m_Visibility = isVisible;
}
