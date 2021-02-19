#include "ui/inspector_widget.hpp"
#include "trackers/shader_manager.hpp"
#include <imgui.h>

using namespace ve;

namespace helper
{
    void drawShader(trackers::ShaderMetadata& shader)
    {
        auto shaderId = std::to_string(shader.m_Id);
        if(ImGui::TreeNode(shaderId.c_str()))
        {
            ImGui::Text(shader.preprocessedSourceCode.c_str());
            ImGui::TreePop();
        }
    }
    void drawProgram(size_t programID, trackers::ShaderProgram& program)
    {
        auto programIDTxt = std::to_string(programID);
        if(ImGui::TreeNode(programIDTxt.c_str()))
        {
            ImGui::Text("Program");
            for(auto& [id, shader]: program.shaders.getMap())
            {
                drawShader(*shader);
            }
            ImGui::TreePop();
        }
    }
    
}

InspectorWidget::InspectorWidget(trackers::ShaderManager& manager)
    : interface(manager)
{
}

void InspectorWidget::onDraw()
{
    ImGui::Begin("Inspector");
    auto shadersCount = std::to_string(interface.shaders.size());
    ImGui::Text(shadersCount.c_str());
    for(auto& [id,shader]: interface.getMap())
    {
        helper::drawProgram(id,*shader);
    }
    ImGui::End();
}


