#include "ui/inspector_widget.hpp"
#include "trackers/shader_tracker.hpp"
#include <imgui.h>
#include <sstream>

using namespace ve;

namespace helper
{
    inline const char* getAsString(bool value)
    {
        if(value)
            return "true";
        return "false";
    }

    void drawShader(trackers::ShaderMetadata& shader)
    {
        auto shaderId = std::to_string(shader.m_Id);
        std::ostringstream title;
        title << shader.getTypeAsString() << " " << shaderId;
        if(ImGui::TreeNode(title.str().c_str()))
        {
            ImGui::Text(shader.preprocessedSourceCode.c_str());
            ImGui::TreePop();
        }
    }

    void tableLine(const char* first, const char* second)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(first);
        ImGui::TableNextColumn();
        ImGui::Text(second);
    }

    void tableLine(const char* first, bool second)
    {
        tableLine(first,getAsString(second));
    }

    void drawProgram(size_t programID, trackers::ShaderProgram& program)
    {
        std::ostringstream title;
        title << "Program with ID " << programID << " ";
        if(ImGui::TreeNode(title.str().c_str()))
        {
            if(program.m_Metadata)
            {
                const auto& meta = *program.m_Metadata;
                ImGui::BeginTable("Metadata",2);
                if(meta.hasDetectedTransformation())
                {
                    tableLine("Using transform:", meta.m_TransformationMatrixName.c_str());
                } else {
                    ImGui::Text("No transform detected");
                }
                tableLine("Is clipspace:", meta.m_IsGeometryShaderUsed);
                tableLine("Is ftransform():", meta.m_HasAnyFtransform);
                tableLine("Is GeometryShader():", meta.m_IsGeometryShaderUsed);
                ImGui::EndTable();
            }
            for(auto& [id, shader]: program.shaders.getMap())
            {
                drawShader(*shader);
            }
            ImGui::TreePop();
        }
    }
    
}

InspectorWidget::InspectorWidget(trackers::ShaderTracker& manager)
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

    ImGui::ShowDemoWindow();
    ImGui::End();
}


