#ifndef VE_SETTINGS_WIDGET_HPP
#define VE_SETTINGS_WIDGET_HPP

#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace ve
{
    class SettingsWidget;
}
namespace ve
{
    template<typename T>
    class SettingsModelScalar
    {
        public:
            virtual ~SettingsModelScalar() = default;
            using callbackType = std::function<void(T)>;
            explicit SettingsModelScalar() = default;
            inline void setValue(T value) { m_Value = value;}
            inline T getValue() { return m_Value;}
            inline void setCallback(callbackType callback) { onValueChanged = callback;}
        protected:
            T m_Value = T();
            callbackType onValueChanged;
    };

    class SettingsWidgetItemBase
    {
        public:
            virtual ~SettingsWidgetItemBase() = default;
            void draw();
            friend class SettingsWidget;
        protected:
            virtual void drawContent() = 0;
            std::string name;
            std::string tooltip;
    };

    template<typename T>
    class SettingsWidgetInputItem: public SettingsWidgetItemBase, public SettingsModelScalar<T>
    {
        protected:
            virtual void drawContent() override;
    };

    template<typename T>
    class SettingsWidgetSliderItem: public SettingsWidgetItemBase, public SettingsModelScalar<T>
    {
        public:
        SettingsWidgetSliderItem(T min, T max): m_Minimum(min), m_Maximum(max) {}
        protected:
            virtual void drawContent() override;

            T m_Minimum = T();
            T m_Maximum = T();
    };
    /**
     * @brief Adapts DearImgui into project
     */
    class SettingsWidget
    {
        public:
            SettingsWidget();
            void draw();

            template <typename T>
            std::shared_ptr<SettingsWidgetInputItem<T>> registerInputItem(typename SettingsWidgetInputItem<T>::callbackType callback, const std::string name, const std::string tooltip = std::string());

            template <typename T>
            std::shared_ptr<SettingsWidgetSliderItem<T>> registerSliderItem(typename SettingsWidgetSliderItem<T>::callbackType callback, const std::string name, T min, T max, const std::string tooltip = std::string());
        private:
            std::vector<std::shared_ptr<SettingsWidgetItemBase>> items;
    };

template <typename T>
std::shared_ptr<SettingsWidgetInputItem<T>> SettingsWidget::registerInputItem(typename SettingsWidgetInputItem<T>::callbackType callback, const std::string name, const std::string tooltip)
{
    auto item = std::make_shared<SettingsWidgetInputItem<T>>();
    item->name = name;
    item->tooltip = tooltip;
    item->setCallback(callback);
    items.push_back(item);
    return item;
}
template <typename T>
std::shared_ptr<SettingsWidgetSliderItem<T>> SettingsWidget::registerSliderItem(typename SettingsWidgetSliderItem<T>::callbackType callback, const std::string name, T min, T max, const std::string tooltip)
{
    auto item = std::make_shared<SettingsWidgetSliderItem<T>>(min, max);
    item->name = name;
    item->tooltip = tooltip;
    item->setCallback(callback);
    items.push_back(item);
    return item;
}

} // namespace ve

#endif
