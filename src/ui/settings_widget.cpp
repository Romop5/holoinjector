#include "settings_widget.hpp"
#include <imgui.h>

using namespace ve;

template<>
void SettingsWidgetInputItem<float>::drawContent()
{
    if(ImGui::InputFloat(name.c_str(), &m_Value) && onValueChanged)
        onValueChanged(m_Value);
}

template<>
void SettingsWidgetInputItem<bool>::drawContent()
{
    if(ImGui::Checkbox(name.c_str(), &m_Value) && onValueChanged)
        onValueChanged(m_Value);
}

template<>
void SettingsWidgetInputItem<void*>::drawContent()
{
    if(ImGui::Button(name.c_str()) && onValueChanged)
        onValueChanged(m_Value);
}


template<>
void SettingsWidgetSliderItem<float>::drawContent()
{
    if(ImGui::SliderFloat(name.c_str(), &m_Value, m_Minimum, m_Maximum) && onValueChanged)
        onValueChanged(m_Value);
}

template<>
void SettingsWidgetSliderItem<int>::drawContent()
{
    if(ImGui::SliderInt(name.c_str(), &m_Value, m_Minimum, m_Maximum) && onValueChanged)
        onValueChanged(m_Value);
}


void SettingsWidgetItemBase::draw()
{
    // Render name / tooltip
    drawContent();
    if(tooltip.size() && ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(450.0f);
        ImGui::TextUnformatted(tooltip.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    // End render
}

//-----------------------------------------------------------------------------

SettingsWidget::SettingsWidget()
{
}

void SettingsWidget::draw()
{
    for(auto& item: items)
    {
        item->draw();
    }
}

std::shared_ptr<SettingsWidgetItemBase> SettingsWidget::getItemByName(const std::string name)
{
    auto it = std::find_if(items.begin(), items.end(), [name](auto item)
    {
        return item->name == name;
    });
    return (it != items.end()?*it:nullptr);
}



