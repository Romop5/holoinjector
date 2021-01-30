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
void SettingsWidgetSliderItem<float>::drawContent()
{
    if(ImGui::SliderFloat(name.c_str(), &m_Value, m_Minimum, m_Maximum) && onValueChanged)
        onValueChanged(m_Value);
}


void SettingsWidgetItemBase::draw()
{
    // Render name / tooltip
    drawContent();
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


