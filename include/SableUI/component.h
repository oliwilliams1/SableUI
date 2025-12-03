#pragma once
#include <SableUI/element.h>
#include <SableUI/memory.h>
#include <SableUI/events.h>
#include <SableUI/renderer.h>
#include <SableUI/utils.h>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

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
                [](void* dst, const void* src)
                {
                    *static_cast<T*>(dst) = *static_cast<const T*>(src);
                });
        }

        void CopyFrom(const StateBlock& other) const
            { copier(ptr, other.ptr); }
    };

    class Window;
    class BaseComponent
    {
    public:
        BaseComponent(Colour colour = Colour{ 32, 32, 32 });
        ~BaseComponent();
        static int GetNumInstances();

        virtual void Layout() {};
        void LayoutWrapper();
        virtual void OnHover() {};
        void BackendInitialisePanel(RendererBackend* renderer);
        void BackendInitialiseChild(const char* name, BaseComponent* parent, const ElementInfo& info);

        template<typename T, typename... Args>
        T* AddComponent(Args&&... args);

        virtual Element* GetRootElement();
        void SetRootElement(Element* element) { rootElement = element; }
        int GetNumChildren() const { return m_childCount; }
        bool Rerender(bool* hasContentsChanged = nullptr);

        bool needsRerender = false;
        virtual void comp_PropagateEvents(const UIEventContext& ctx);
        virtual bool comp_PropagateComponentStateChanges(bool* hasContentsChanged = nullptr);

        virtual void OnUpdate(const UIEventContext& ctx) {}; // user override

        template<typename T>
        void RegisterState(T* variable)
            { m_stateBlocks.push_back(StateBlock::Create(variable)); }

        template<typename T>
        void RegisterReference(T* variable)
            { m_stateBlocks.push_back(StateBlock::Create(variable)); }

        void RegisterQueue(CustomTargetQueue** queuePtrAddr)
            { m_customTargetQueuePtrs.push_back(queuePtrAddr); }

        void CopyStateFrom(const BaseComponent& other);

        Element* GetElementById(const SableString& id);
    
        std::vector<BaseComponent*> m_componentChildren;

    protected:
        Element* rootElement = nullptr;
        std::vector<StateBlock> m_stateBlocks;
        std::vector<CustomTargetQueue**> m_customTargetQueuePtrs;

    private:
        size_t m_hash = 0;
        RendererBackend* m_renderer = nullptr;
        Colour m_bgColour = Colour{ 32, 32, 32 };
        int m_childCount = 0;
    };

    template<typename T, typename... Args>
    T* BaseComponent::AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

        T* component;

        if (static_cast<size_t>(m_childCount) < m_componentChildren.size())
        {
            component = static_cast<T*>(m_componentChildren[m_childCount]);
        }
        else
        {
            component = SableMemory::SB_new<T>(std::forward<Args>(args)...);
            m_componentChildren.push_back(component);
        }

        m_childCount++;
        return component;
    }
}