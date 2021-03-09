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

    inline void tableLine(const char* first, const char* second)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(ImColor(0.9f,0.9f,0.9f),first);
        ImGui::TableNextColumn();
        ImGui::Text(second);
    }

    inline void tableLine(const char* first, bool& second)
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

    inline void tableLineInfo(const char* first, bool second)
    {
        ImVec4 boolColor = (second?ImColor(0.0f,1.0f,0.0f):ImColor(1.0f,0.0f,0.0f));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(ImColor(0.9f,0.9f,0.9f),first);
        ImGui::TableNextColumn();
        ImGui::TextColored(boolColor,getAsString(second));
        ImGui::SameLine();
    }



    inline void drawShader(trackers::ShaderMetadata& shader)
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

    inline void drawProgram(size_t programID, trackers::ShaderProgram& program)
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

    inline std::string physicalTypeToString(trackers::TextureType type)
    {
        switch(type)
        {
            case trackers::TextureType::RENDERBUFFER:
                return "RENDERBUFFER";
            case trackers::TextureType::TEXTURE:
                return "TEXTURE";
            default:
                return "UNKNOWN";
        }
    }

    inline void drawFBO(size_t fboID, trackers::FramebufferMetadata& fbo)
    {
        std::ostringstream title;
        title << "FBO ID " << fboID << " ";
        if(ImGui::TreeNode(title.str().c_str()))
        {
            if(ImGui::BeginTable("Metadata",2))
            {
                tableLineInfo("Is shadow map:", fbo.isShadowMapFBO());
                tableLineInfo("Is enviromental map:", fbo.isEnvironmentMapFBO());
                tableLineInfo("Is layered rendering:", fbo.isLayeredRendering());
                ImGui::EndTable();
            }

            for(auto& [type,attachment]: fbo.getAttachmentMap().getMap())
            {
                auto& texture = attachment.texture;
                if(ImGui::BeginTable("Metadata",2))
                {
                    tableLine("Attachment:", (trackers::FramebufferMetadata::getAttachmentTypeAsString(type)).c_str());
                    tableLine("Physical type:", physicalTypeToString(texture->getPhysicalTextureType()).c_str());
                    tableLine("Type:", texture->getTypeAsString(texture->getType()).c_str());
                    tableLine("Resolution:", (std::to_string(texture->getWidth())+"x"+std::to_string(texture->getHeight())).c_str());
                    tableLine("Format:", texture->getFormatAsString(texture->getFormat()).c_str());
                    ImGui::EndTable();
                }
                ImGui::Image((void*)(intptr_t) texture->getID(), ImVec2(50,50));
                if(texture->hasShadowTexture())
                {
                    ImGui::SameLine();
                    ImGui::Image((void*)(intptr_t) texture->getShadowedTextureId(), ImVec2(50,50));
                }
            }
            ImGui::TreePop();
        }
        ImGui::Separator();
    }

    inline void drawLoggerSettings(Logger& logger)
    {
        const auto options = std::vector<std::pair<Logger::LogLevel, std::string>>{
            { Logger::LogLevel::ERROR_LOG, "Error"},
            { Logger::LogLevel::INFO_LOG, "Info"},
            { Logger::LogLevel::DEBUG_LOG, "Debug"},
            { Logger::LogLevel::DEBUG_PER_FRAME_LOG, "Debug - per frame"},
        };

        if(ImGui::BeginCombo("Output log verbosity", "Maximum level"))
        {
            size_t item_current_idx = static_cast<size_t>(logger.getMaximumLevel());
            for (int n = 0; n < options.size(); n++)
            {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(options[n].second.c_str(), is_selected))
                {
                    item_current_idx = n;
                    logger.setMaximumLevel(options[n].first);
                    Logger::logDebug("Changed maximum log level to: ", options[n].second);
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

}

InspectorWidget::InspectorWidget(trackers::ShaderTracker& manager, trackers::FramebufferTracker& fbo, trackers::TextureTracker& textureTracker)
    : shaderInterface(manager), interfaceFBO(fbo), interfaceTextureTracker(textureTracker)
{
}

void InspectorWidget::onDraw()
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(2000,1000));
    ImGui::Begin("Inspector");

    if(ImGui::Button("Delete shadow {textures, FBOs}"))
    {
        for(auto& [id, fbo]: interfaceFBO.getMap())
        {
            fbo->freeShadowedFBO();
        }
        for(auto& [id, texture]: interfaceTextureTracker.getMap())
        {
            texture->freeShadowedTexture();
        }
    }
    if(ImGui::Button("Delete shadow FBOs"))
    {
        for(auto& [id, fbo]: interfaceFBO.getMap())
        {
            fbo->freeShadowedFBO();
        }
    }

    if(ImGui::BeginTabBar("Inspectables"))
    {
        if(ImGui::BeginTabItem("Settings"))
        {
            helper::drawLoggerSettings(Logger::getInstance());
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Programs"))
        {
            auto shadersCount = std::to_string(shaderInterface.shaders.size());
            for(auto& [id,shader]: shaderInterface.getMap())
            {
                helper::drawProgram(id,*shader);
            }
            ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("FBOs"))
        {
            for(auto& [id,fbo]: interfaceFBO.getMap())
            {
                helper::drawFBO(id,*fbo);
            }
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
    ImGui::End();
}


