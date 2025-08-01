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

        virtual void Init() {};
        void BackendInitialise(Renderer* renderer);

        Element* GetRootElement();

    protected:
        Element rootElement;
    };
}