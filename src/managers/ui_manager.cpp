#include "managers/ui_manager.hpp"

#include "context.hpp"
#include "ui/settings_widget.hpp"
#include "ui/inspector_widget.hpp"
#include "ui/x11_sniffer.hpp"

#include "imgui_adapter.hpp"
#include "pipeline/output_fbo.hpp"
#include "pipeline/camera_parameters.hpp"
#include "pipeline/virtual_cameras.hpp"
#include <X11/keysym.h>

using namespace ve;
using namespace ve::managers;


UIManager::UIManager() = default;
UIManager::~UIManager() = default;

void UIManager::initialize(Context& context)
{
    inspectorWidget = std::make_unique<InspectorWidget>(context.getManager(), context.getFBOTracker(),context.getTextureTracker());
    registerCallbacks(context);
}

void UIManager::deinitialize(Context& context)
{
    inspectorWidget.reset();
    context.getSettingsWidget().freeItems();
}

void UIManager::registerCallbacks(Context& context)
{
    // Register settings UI
    context.getSettingsWidget().registerInputItem<bool>([this,&context](auto newValue)
    {
        context.getX11Sniffer().turnFullscreen();
    }, "Toggle fullscreen")->setValue(false);

    context.getSettingsWidget().registerSliderItem<float>([this,&context](auto newValue)
    {
        context.getGui().setScaling(newValue);
    }, "UI font scaling",0.5, 5, "Scale IMGUI window to fit high DPI displays")->setValue(1.0);

    context.getSettingsWidget().registerSliderItem<float>([this,&context](auto newValue)
    {
        context.getCameraParameters().m_frontOpticalAxisCentreDistance = newValue;
    }, "Near plane",0.0, 40, "Define distance of near plane");

    context.getSettingsWidget().registerSliderItem<float>([this,&context](auto newValue)
    {
        context.getCameraParameters().m_XShiftMultiplier = newValue;
    }, "Horizontal shift",0.0, 8, "Define horizontal distance of left-most side view from the original point of view");

    context.getSettingsWidget().registerInputItem<bool>([this,&context](auto newValue)
    {
        context.getOutputFBO().toggleGridView();
    }, "Toggle quilt/native format", "Toggle between transformed native format, suitable for 3D Displays, and quilt format, showing all views into scene in a grid.");

    context.getSettingsWidget().registerInputItem<bool>([this,&context](auto newValue)
    {
        context.getOutputFBO().toggleSingleViewGridView();
    }, "Toggle single view vs quilt view", "Toggle between quilt grid and single view");
    context.getSettingsWidget().registerSliderItem<int>([this,&context](auto newValue)
    {
        context.getOutputFBO().setOnlyQuiltImageID(newValue);
    }, "Single view ID",0, 45, "Select one of quilt views");

    const auto params = context.getOutputFBO().getHoloDisplayParameters();
    auto pitchItem = context.getSettingsWidget().registerSliderItem<float>([this,&context](auto newValue)
    {
        auto params = context.getOutputFBO().getHoloDisplayParameters();
        params.m_Pitch = newValue;
        context.getOutputFBO().setHoloDisplayParameters(params);
    }, "Pitch",0.0, 400, "");
    pitchItem->setValue(params.m_Pitch);

    auto tiltItem = context.getSettingsWidget().registerSliderItem<float>([this,&context](auto newValue)
    {
        auto params = context.getOutputFBO().getHoloDisplayParameters();
        params.m_Pitch = newValue;
        context.getOutputFBO().setHoloDisplayParameters(params);
    }, "Tilt",-1.0, 1, "");
    tiltItem->setValue(params.m_Tilt);

    auto centerItem = context.getSettingsWidget().registerSliderItem<float>([this,&context](auto newValue)
    {
        auto params = context.getOutputFBO().getHoloDisplayParameters();
        params.m_Center = newValue;
        context.getOutputFBO().setHoloDisplayParameters(params);
    }, "Center",-1.0, 1, "");
    centerItem->setValue(params.m_Center);

    context.getSettingsWidget().registerInputItem<void*>([this,&context,params,pitchItem, tiltItem, centerItem] (auto)
    {
        pitchItem->setValue(params.m_Pitch);
        tiltItem->setValue(params.m_Tilt);
        centerItem->setValue(params.m_Center);
        context.getOutputFBO().setHoloDisplayParameters(params);
    }, "Reset default holo parameters","");

    context.getSettingsWidget().registerInputItem<void*>([this,&context] (auto)
    {
        context.getGui().setVisibility(!context.getGui().isVisible());
    }, "Hide GUI","");
}

void UIManager::onKeyPressed(Context& context, size_t keySym)
{
    const float increment = 0.10f;
    switch(keySym)
    {
        case XK_F1: case XK_F2:
            context.getCameraParameters().m_XShiftMultiplier += (keySym == XK_F1?1.0:-1.0)*increment;
        break;
        case XK_F3: case XK_F4:
            context.getCameraParameters().m_frontOpticalAxisCentreDistance += (keySym == XK_F3?1.0:-1.0)*0.5;
        break; 
        case XK_F5:
            context.getCameraParameters() = ve::pipeline::CameraParameters();
        break;
        case XK_F11:
            context.getGui().setVisibility(!context.getGui().isVisible());
        break;
        case XK_F12:
            context.m_IsMultiviewActivated = !context.m_IsMultiviewActivated;
        break;
        case XK_F10:
            context.getX11Sniffer().turnFullscreen();
        break;
        default:
        break;
    }
    Logger::log("[Repeater] Setting: frontDistance (",context.getCameraParameters().m_frontOpticalAxisCentreDistance, "), X multiplier(",
            context.getCameraParameters().m_XShiftMultiplier, ")");
    context.getCameras().updateParamaters(context.getCameraParameters());
}

void UIManager::onDraw(Context& context)
{
    context.getSettingsWidget().draw();
    if(inspectorWidget)
    {
        inspectorWidget->onDraw();
    }
}


