#pragma once
#include <SableUI/core/renderer.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <concepts>
#include <vector>
#include <string>

namespace SableUI
{
    class StateBase
    {
    public:
        virtual ~StateBase() = default;
        virtual void Sync(StateBase* other) = 0;
    };

    class Window;
    class BaseComponent
    {
    public:
        BaseComponent(Colour colour = Colour{ 32, 32, 32 });
        static int GetNumInstances();
        virtual ~BaseComponent();

        virtual void Layout() {};
        virtual void OnUpdate(const UIEventContext& ctx) {};
        virtual void OnUpdatePostLayout(const UIEventContext& ctx) {};

        void LayoutWrapper();
        void BackendInitialisePanel();
        void SetRenderer(RendererBackend* renderer) { m_renderer = renderer; }
        void BackendInitialiseChild(const char* name, BaseComponent* parent, const ElementInfo& info);
        void Render(int z = 0);

        BaseComponent* AddComponent(const std::string& componentName);

        Element* GetRootElement();
        void SetRootElement(Element* element) { rootElement = element; }
        int GetNumChildren() const { return m_childCount; }
        bool Rerender(bool* hasContentsChanged = nullptr);

        bool needsRerender = false;
        void comp_PropagateEvents(const UIEventContext& ctx);
        bool comp_PropagateComponentStateChanges(bool* hasContentsChanged = nullptr);
        void comp_PropagatePostLayoutEvents(const UIEventContext& ctx);

        void RegisterState(StateBase* state) { m_states.push_back(state); }

        void CopyStateFrom(const BaseComponent& other);
        Element* GetElementById(const SableString& id);

        std::vector<BaseComponent*> m_componentChildren;
        void RegisterHoverElement(Element* el);

    protected:
        std::vector<BaseComponent*> m_garbageChildren;
        std::vector<StateBase*> m_states;
        std::vector<Element*> m_hoverElements;

        void UpdateHoverStyling(const UIEventContext& ctx);

    private:
        UIEventContext m_lastEventCtx;
        Element* rootElement = nullptr;
        size_t m_hash = 0;
        RendererBackend* m_renderer = nullptr;
        Colour m_bgColour = Colour{ 32, 32, 32 };
        int m_childCount = 0;
    };

    void _priv_comp_PostEmptyEvent();

    template<typename T>
    concept HasEqualityOperator = requires(const T & a, const T & b) {
        { a == b } -> std::same_as<bool>;
    };

    template<typename T>
    class State : public StateBase {
        static_assert(HasEqualityOperator<T>, "State<T> requires operator== overloaded");

    public:
        State(BaseComponent* owner, T initialValue)
            : m_value(initialValue), m_owner(owner) {
            owner->RegisterState(this);
        }

        const T& get() const { return m_value; }

        void set(const T& newValue) {
            if (m_value == newValue) return;
            m_value = newValue;
            m_owner->needsRerender = true;
            _priv_comp_PostEmptyEvent();
        }

        void Sync(StateBase* other) override {
            if (!other) return;
            auto* otherPtr = static_cast<State<T>*>(other);
            this->m_value = otherPtr->m_value;
        }

        operator const T& () const { return m_value; }

        State& operator=(const T& newValue) {
            set(newValue);
            return *this;
        }

    private:
        T m_value;
        BaseComponent* m_owner;
    };

    template<typename T>
    class Ref : public StateBase {
    public:
        Ref(BaseComponent* owner, T initialValue)
            : m_value(initialValue) {
            owner->RegisterState(this);
        }

        void Sync(StateBase* other) override {
            if (!other) return;
            auto* otherPtr = static_cast<Ref<T>*>(other);
            this->m_value = otherPtr->m_value;
        }

        T& get() { return m_value; }

        void set(const T& newValue) {
            m_value = newValue;
        }

        operator const T& () const { return m_value; }

        Ref& operator=(const T& newValue) {
            m_value = newValue;
            return *this;
        }

    private:
        T m_value;
    };
}