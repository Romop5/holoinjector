#include "ui/inspector_widget.hpp"
#include "trackers/shader_tracker.hpp"
#include "trackers/framebuffer_tracker.hpp"
#include "trackers/texture_tracker.hpp"
#include <imgui.h>
#include <sstream>
#include <string>

using namespace ve;

namespace helper
{
    inline const char* getAsString(bool value)
    {
        if(value)
            return "true";
        return "false";
    }

    void tableLine(const char* first, const char* second)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(ImColor(0.9f,0.9f,0.9f),first);
        ImGui::TableNextColumn();
        ImGui::Text(second);
    }

    void tableLine(const char* first, bool& second)
    {
        ImVec4 boolColor = (second?ImColor(0.0f,1.0f,0.0f):ImColor(1.0f,0.0f,0.0f));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(ImColor(0.9f,0.9f,0.9f),first);
        ImGui::TableNextColumn();
        ImGui::TextColored(boolColor,getAsString(second));
        ImGui::SameLine();
        ImGui::PushID(&second);
        if(ImGui::Button("Toggle"))
        {
            second = !second;
        }
        ImGui::PopID();
    }

    void tableLineInfo(const char* first, bool second)
    {
        ImVec4 boolColor = (second?ImColor(0.0f,1.0f,0.0f):ImColor(1.0f,0.0f,0.0f));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(ImColor(0.9f,0.9f,0.9f),first);
        ImGui::TableNextColumn();
        ImGui::TextColored(boolColor,getAsString(second));
        ImGui::SameLine();
    }



    void drawShader(trackers::ShaderMetadata& shader)
    {
        auto shaderId = std::to_string(shader.m_Id);
        std::ostringstream title;
        title << shader.getTypeAsString() << " " << shaderId;
        if(ImGui::TreeNode(title.str().c_str()))
        {
            ImGui::BeginTable("Metadata",2);
            tableLine("Hash: ", std::to_string(std::hash<std::string>()(shader.preprocessedSourceCode)).c_str());
            ImGui::EndTable();
            ImGui::Text(shader.preprocessedSourceCode.c_str());
            ImGui::TreePop();
        }
    }

    void drawProgram(size_t programID, trackers::ShaderProgram& program)
    {
        std::ostringstream title;
        title << "Program with ID " << programID << " ";
        if(ImGui::TreeNode(title.str().c_str()))
        {
            if(program.m_Metadata)
            {
                auto& meta = *program.m_Metadata;
                ImGui::BeginTable("Metadata",2);
                tableLine("Is invisible:", meta.m_IsInvisible);
                tableLine("Is injected:", meta.m_IsInjected);
                tableLine("Is linked:", meta.m_IsLinkedCorrectly);
                tableLine("Is GeometryShader():", meta.m_IsGeometryShaderUsed);
                if(meta.hasDetectedTransformation())
                {
                    tableLine("Using transform:", meta.m_TransformationMatrixName.c_str());
                } else {
                    tableLine("Using transform: ", "No transform");
                }
                ImGui::Separator();
                tableLine("Is clipspace:", meta.m_IsClipSpaceTransform);
                tableLine("Is ftransform():", meta.m_HasAnyFtransform);
                ImGui::EndTable();
            }
            for(auto& [id, shader]: program.shaders.getMap())
            {
                drawShader(*shader);
            }
            ImGui::TreePop();
        }
        ImGui::Separator();
    }

    void drawFBO(size_t fboID, trackers::FramebufferMetadata& fbo)
    {
        std::ostringstream title;
        title << "Program with ID " << fboID << " ";
        if(ImGui::TreeNode(title.str().c_str()))
        {
            ImGui::BeginTable("Metadata",2);
            tableLineInfo("Is shadow map:", fbo.isShadowMapFBO());
            tableLineInfo("Is enviromental map:", fbo.isEnvironmentMapFBO());
            tableLineInfo("Is layered rendering:", fbo.isLayeredRendering());

            for(auto& [type,attachment]: fbo.getAttachmentMap().getMap())
            {
                tableLine("Attachment:", (trackers::FramebufferMetadata::getAttachmentTypeAsString(type) + " - " + attachment.texture->getTypeAsString(attachment.texture->getType())).c_str());
            }
            ImGui::EndTable();
            ImGui::TreePop();
        }
        ImGui::Separator();
    }

}

InspectorWidget::InspectorWidget(trackers::ShaderTracker& manager, trackers::FramebufferTracker& fbo)
    : shaderInterface(manager), interfaceFBO(fbo)
{
}

void InspectorWidget::onDraw()
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(2000,1000));
    ImGui::Begin("Inspector");
    auto shadersCount = std::to_string(shaderInterface.shaders.size());
    ImGui::Text(shadersCount.c_str());
    for(auto& [id,shader]: shaderInterface.getMap())
    {
        helper::drawProgram(id,*shader);
    }
    ImGui::End();

    ImGui::SetNextWindowSizeConstraints(ImVec2(800, 400), ImVec2(2000,1000));
    ImGui::Begin("InspectorFBO");
    for(auto& [id,fbo]: interfaceFBO.getMap())
    {
        helper::drawFBO(id,*fbo);
    }
    ImGui::End();
}


