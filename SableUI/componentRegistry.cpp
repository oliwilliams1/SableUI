#include <SableUI/componentRegistry.h>
#include <SableUI/component.h>
#include <SableUI/console.h>
#include <SableUI/panel.h>
#include <SableUI/memory.h>
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
    m_component->BackendInitialisePanel(m_renderer);

    Update();
    return comp;
}

BaseComponent* BaseComponent::AddComponent(const std::string& componentName)
{
    BaseComponent* component = ComponentRegistry::GetInstance().Create(componentName);

    if (!component)
    {
        SableUI_Runtime_Error("Cannot attach component: %s", componentName.c_str());
        return nullptr;
    }

    if (static_cast<size_t>(m_childCount) < m_componentChildren.size())
    {
        BaseComponent* existing = m_componentChildren[m_childCount];

        if (existing)
            m_garbageChildren.push_back(existing);

        m_componentChildren[m_childCount] = component;
    }
    else
    {
        m_componentChildren.push_back(component);
    }

    m_childCount++;
    return component;
}

#include <SableUI/components/menuBar.h>
#include <SableUI/components/debugComponents.h>
#include <SableUI/components/tabStack.h>
#include <SableUI/components/scrollView.h>

void SableUI::RegisterSableUIComponents()
{
    RegisterComponent<MenuBar>("Menu Bar");
    RegisterComponent<_TabStackDef>("TabStack");
    RegisterComponent<ElementTreeView>("ElementTreeView");
    RegisterComponent<MemoryDebugger>("MemoryDebugger");
    RegisterComponent<PropertiesView>("PropertiesView");
    RegisterComponent<ScrollView>("ScrollView");
}
