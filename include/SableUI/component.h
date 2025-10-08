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
        StateSetter(
            std::function<void(const T&)> setter, 
            std::function<void()> initialiser)
            : m_setter(setter) { initialiser(); }

        void operator()(const T& value) const { m_setter(value); }
        void set(const T& value) const { m_setter(value); }

        StateSetter(const StateSetter& other) = delete;
        StateSetter& operator=(const StateSetter& other) = delete;

    private:
        std::function<void(const T&)> m_setter;
    };

    class Element;

    struct StateBlock
    {
        void* start;
        size_t size;
        StateBlock(void* start, size_t size) : start(start), size(size) {}

        void operator=(const StateBlock& other)
        {
            if (this != &other)
            {
                if (other.size != size)
                {
                    SableUI_Error("StateBlock size mismatch, potential leak");
                    return;
                }
                std::memcpy(start, other.start, size);
            }
        };
    };

    class BaseComponent
    {
    public:
        BaseComponent(Colour colour = Colour{ 32, 32, 32 });
        ~BaseComponent();

        virtual void Layout() {};
        void LayoutWrapper() { m_childCount = 0; Layout(); }
        virtual void OnHover() {};
        void BackendInitialisePanel(Renderer* renderer);
        void BackendInitialiseChild(const char* name, BaseComponent* parent, const ElementInfo& info);

        template<typename T, typename... Args>
        BaseComponent* AddComponent(Args&&... args);

        Element* GetRootElement();
        void SetRootElement(Element* element) { rootElement = element; }
        int GetNumChildren() const { return m_childCount; }
        bool Rerender();

        bool needsRerender = false;
        bool comp_PropagateComponentStateChanges();

        void AddStateBlock(void* start, size_t size) { m_stateBlocks.push_back(StateBlock(start, size)); }

        std::vector<StateBlock> m_stateBlocks; 

    protected:
        Element* rootElement = nullptr;

    private:
        size_t m_hash = 0;
        Renderer* m_renderer = nullptr;
        Colour m_bgColour = Colour{ 32, 32, 32 };
        std::vector<BaseComponent*> m_componentChildren;
        int m_childCount = 0;
    };

    template<typename T, typename ...Args>
    inline BaseComponent* BaseComponent::AddComponent(Args && ...args)
    {
        static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

        T* component = new T(std::forward<Args>(args)...);
        m_componentChildren.push_back(component);
        m_childCount++;
        return component;
    }
}