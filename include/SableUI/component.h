#pragma once
#include "SableUI/renderer.h"
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

        bool needsRerender = false;
        bool comp_PropagateComponentStateChanges();

    protected:
        Element* rootElement = nullptr;

    private:
        Renderer* m_renderer = nullptr;
        Colour m_bgColour = Colour{ 32, 32, 32 };
    };
}