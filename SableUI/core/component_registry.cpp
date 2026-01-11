#include <SableUI/core/component_registry.h>
#include <SableUI/core/component.h>
#include <SableUI/core/panel.h>
#include <SableUI/core/floating_panel.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/console.h>
#include <string>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "ComponentRegistry"

using namespace SableUI;

ComponentRegistry& ComponentRegistry::GetInstance()
{
    static ComponentRegistry instance;
    return instance;
}

BaseComponent* ComponentRegistry::Create(const std::string& name) const
{
    auto it = m_factories.find(name);
    if (it == m_factories.end())
    {
        SableUI_Runtime_Error("Component %s is not registered", name.c_str());
        return nullptr;
    }

    return it->second();
}

bool ComponentRegistry::IsRegistered(const std::string& name) const
{
    return m_factories.find(name) != m_factories.end();
}

void ComponentRegistry::Shutdown()
{
    GetInstance().m_factories.clear();
}

BaseComponent* ContentPanel::AttachComponent(const std::string& componentName)
{
    if (m_component != nullptr)
        SableMemory::SB_delete(m_component);

    BaseComponent* comp = ComponentRegistry::GetInstance().Create(componentName);

    if (!comp)
    {
        SableUI_Runtime_Error("Cannot attach component %s", componentName.c_str());
        return nullptr;
    }

    m_component = comp;
    m_component->SetRenderer(m_renderer);
    
    return comp;
}

BaseComponent* FloatingPanel::AttachComponent(const std::string& componentName)
{
    if (m_component != nullptr)
        SableMemory::SB_delete(m_component);

    BaseComponent* comp = ComponentRegistry::GetInstance().Create(componentName);

    if (!comp)
    {
        SableUI_Runtime_Error("Cannot attach component %s", componentName.c_str());
        return nullptr;
    }

    m_component = comp;
    m_component->SetRenderer(m_renderer);

    return comp;
}

BaseComponent* BaseComponent::AddComponent(const std::string& componentName)
{
    BaseComponent* component =
        ComponentRegistry::GetInstance().Create(componentName);

    if (!component)
    {
        SableUI_Runtime_Error("Cannot attach component: %s", componentName.c_str());
        return nullptr;
    }

    return AttachComponent(component);
}

#include <SableUI/components/debug_components.h>
#include <SableUI/components/button.h>
#include <SableUI/components/checkbox.h>
#include <SableUI/components/text_field.h>

void SableUI::RegisterSableUIComponents()
{
    RegisterComponent<LayoutDebugger>("LayoutDebugger");
    RegisterComponent<MemoryDebugger>("MemoryDebugger");
    RegisterComponent<PropertiesPanel>("PropertiesPanel");
    
    RegisterComponent<Button>("sableui_button");
    RegisterComponent<Checkbox>("sableui_checkbox");
    RegisterComponent<TextFieldComponent>("sableui_text_field");
}
