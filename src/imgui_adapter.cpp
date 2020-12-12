#include "imgui_adapter.hpp"
#include <imgui_impl_opengl3.h>
#include "context.hpp"

using namespace ve;

bool ve::ImguiAdapter::initialize()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
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
