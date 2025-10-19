#pragma once
#include "SableUI/element.h"
#include "SableUI/memory.h"
#include <functional>
#include <map>
#include <any>
#include <tuple>

namespace SableUI
{
    template<typename T>
    struct StateSetter
    {
        StateSetter(std::function<void(const T&)> setter)
            : m_setter(setter) {};

        void operator()(const T& value) const { m_setter(value); }
        void set(const T& value) const { m_setter(value); }

        StateSetter(const StateSetter& other) = delete;
        StateSetter& operator=(const StateSetter& other) = delete;

    private:
        std::function<void(const T&)> m_setter;
    };

    class Element;

    class StateBlock
    {
        StateBlock(void* ptr, std::function<void(void*, const void*)> copier) : ptr(ptr), copier(copier) {}
        
        void* ptr;
        std::function<void(void*, const void*)> copier;
    
    public:
        template<typename T>
        static StateBlock Create(T* variable)
        {
            return StateBlock(
                variable, 
                [](void* dst, const void* src) {
                    *static_cast<T*>(dst) = *static_cast<const T*>(src);
                });
        }

        void CopyFrom(const StateBlock& other) const
        { copier(ptr, other.ptr); }
    };

    class BaseComponent
    {
    public:
        BaseComponent(Colour colour = Colour{ 32, 32, 32 });

        virtual ~BaseComponent();

        virtual void Layout() {};
        void LayoutWrapper();
        virtual void OnHover() {};
        void BackendInitialisePanel(Renderer* renderer);
        void BackendInitialiseChild(const char* name, BaseComponent* parent, const ElementInfo& info);

        template<typename T, typename... Args>
        BaseComponent* AddComponent(Args&&... args);

        Element* GetRootElement();
        void SetRootElement(Element* element) { rootElement = element; }
        int GetNumChildren() const { return m_childCount; }
        bool Rerender(bool* hasContentsChanged = nullptr);

        bool needsRerender = false;
        void comp_PropagateEvents(const UIEventContext& ctx);
        bool comp_PropagateComponentStateChanges(bool* hasContentsChanged = nullptr);

        virtual void OnUpdate(const UIEventContext& ctx) {}; // user override

        template<typename T>
        void RegisterState(T* variable)
            { m_stateBlocks.push_back(StateBlock::Create(variable)); }

        void CopyStateFrom(const BaseComponent& other);

    protected:
        std::vector<StateBlock> m_stateBlocks;
        Element* rootElement = nullptr;

    private:
        size_t m_hash = 0;
        Renderer* m_renderer = nullptr;
        Colour m_bgColour = Colour{ 32, 32, 32 };
        std::vector<BaseComponent*> m_componentChildren;
        int m_childCount = 0;
    };

    template<typename T, typename... Args>
    BaseComponent* BaseComponent::AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

        T* component = SableMemory::SB_new<T>(std::forward<Args>(args)...);
        m_componentChildren.push_back(component);
        m_childCount++;
        return component;
    }
}