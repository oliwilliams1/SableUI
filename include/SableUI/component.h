#pragma once
#include "SableUI\renderer.h"

namespace SableUI
{
    class Element;

    class BaseComponent
    {
    public:
        BaseComponent(Colour colour = Colour{ 32, 32, 32 });
        ~BaseComponent() = default;

        void SetRenderer(Renderer* renderer);
        virtual void Init() {};

        Element* GetBaseElement();

    protected:
        Element m_baseElement;
    };
}