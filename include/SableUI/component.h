#pragma once
#include "SableUI/element.h"
#include <functional>
#include <map>
#include <any>
#include <tuple>

namespace SableUI
{
    template<typename T>
    struct StateSetter
    {
        StateSetter(std::function<void(const T&)> setter) : m_setter(setter) {}
        void operator()(const T& value) const { m_setter(value); }
        void set(const T& value) const { m_setter(value); }

        StateSetter(const StateSetter& other) = delete;
        StateSetter& operator=(const StateSetter& other) = delete;

    private:
        std::function<void(const T&)> m_setter;
    };

    class Element;

    class BaseComponent
    {
    public:
        BaseComponent(Colour colour = Colour{ 32, 32, 32 });
        ~BaseComponent();

        virtual void Layout() {};
        virtual void OnHover() {};
        void BackendInitialisePanel(Renderer* renderer);
        void BackendInitialiseChild(BaseComponent* parent, const ElementInfo& info);

        template<typename T, typename... Args>
        BaseComponent* AddComponent(Args&&... args);

        Element* GetRootElement();

        void Rerender();

        bool needsRerender = false;
        bool comp_PropagateComponentStateChanges();

    protected:
        Element* rootElement = nullptr;

    private:
        Renderer* m_renderer = nullptr;
        Colour m_bgColour = Colour{ 32, 32, 32 };
        std::vector<BaseComponent*> m_componentChildren;
    };

    template<typename T, typename ...Args>
    inline BaseComponent* BaseComponent::AddComponent(Args && ...args)
    {
        static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

        T* component = new T(std::forward<Args>(args)...);
        m_componentChildren.push_back(component);

        return component;
    }
}