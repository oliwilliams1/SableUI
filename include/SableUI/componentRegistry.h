#pragma once
#include <SableUI/memory.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <type_traits>

namespace SableUI
{
    class BaseComponent;
    class ComponentRegistry
    {
    public:
        static ComponentRegistry& GetInstance();
        static void Shutdown();

        template<typename T>
        void Register(const std::string& name);

        BaseComponent* Create(const std::string& name) const;
        bool IsRegistered(const std::string& name) const;

    private:
        ComponentRegistry() = default;
        ~ComponentRegistry() = default;

        ComponentRegistry(const ComponentRegistry&) = delete;
        ComponentRegistry& operator=(const ComponentRegistry&) = delete;

        std::unordered_map<std::string, std::function<BaseComponent* ()>> m_factories;
    };

    template<typename T>
    void ComponentRegistry::Register(const std::string& name)
    {
        static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

        m_factories[name] = []() -> BaseComponent* {
            return SableMemory::SB_new<T>();
        };
    }

    template<typename T>
    void RegisterComponent(const std::string& name)
    {
        ComponentRegistry::GetInstance().Register<T>(name);
    }
}