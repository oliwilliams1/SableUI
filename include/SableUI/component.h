#pragma once
#include "SableUI\renderer.h"

namespace SableUI
{
    class Element;

    class BaseComponent
    {
    public:
        virtual void Init() {};

        BaseComponent(Renderer* renderer);
        ~BaseComponent() = default;

        Element* GetBaseElement();

    protected:
        Element m_baseElement;
    };
}