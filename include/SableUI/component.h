#pragma once
#include "SableUI\renderer.h"
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

    private:
        std::function<void(const T&)> m_setter;
    };

    class Element;

    class BaseComponent
    {
    public:
        BaseComponent(Colour colour = Colour{ 32, 32, 32 });
        ~BaseComponent() = default;

        virtual void Layout() {};
        virtual void OnHover() {};
        void BackendInitialise(Renderer* renderer);

        Element* GetRootElement();

        void Rerender();

    protected:
        Element rootElement;

        template<typename T>
        std::tuple<T&, StateSetter<T>> useState(const T& initialValue)
        {
            if (m_states.find(m_stateCounter) == m_states.end())
            {
                m_states[m_stateCounter] = initialValue;
            }

            T& stateValue = std::any_cast<T&>(m_states.at(m_stateCounter));

            auto setterLambda = [this, stateIndex = m_stateCounter](const T& newValue) {
                m_states.at(stateIndex) = newValue;
                this->Rerender();
            };

            StateSetter<T> stateSetter(setterLambda);

            m_stateCounter++;

            return { stateValue, stateSetter };
        }

        void resetStateCounter() { m_stateCounter = 0; }

    private:
        std::map<int, std::any> m_states;
        int m_stateCounter = 0;
        Renderer* m_renderer = nullptr;
    };
}