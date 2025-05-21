#pragma once
#include <memory>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"

namespace SableUI
{
    class BaseElement
    {
    public:
        BaseElement() : bgColour({ 255, 0, 0, 0 }) {};

        BaseElement(SableUI::colour col) : bgColour(col) {};

        BaseElement(float w, float h, 
            SableUI::colour col = SableUI::colour(128, 128, 128))
            : width(w), height(h), bgColour(col) {};

        BaseElement(float xOffset, float yOffset, float w, float h, 
            SableUI::colour col = SableUI::colour(128, 128, 128))
            : xOffset(xOffset), yOffset(yOffset), width(w), height(h), bgColour(col) {};

        BaseElement(float w, float h, 
            SableUI::RectType wType, SableUI::RectType hType, SableUI::colour col = SableUI::colour(128, 128, 128))
            : width(w), height(h), wType(wType), hType(hType), bgColour(col) {};

        BaseElement(float xOffset, float yOffset, float w, float h, 
            SableUI::RectType wType, SableUI::RectType hType, SableUI::colour col = SableUI::colour(128, 128, 128))
            : xOffset(xOffset), yOffset(yOffset), width(w), height(h), wType(wType), hType(hType), bgColour(col) {};

        /* Base render (background) */
        void Render();
        /* Virtual render to add additional, custom elements */
        virtual void AdditionalRender() {};

        void SetRect(const SableUI::rect& rect);

        /* user-level settings for rect */
        float xOffset = 0;
        float yOffset = 0;
        float width = 0;
        float height = 0;
        float padding = 0;
        SableUI::RectType wType = SableUI::RectType::FILL;
        SableUI::RectType hType = SableUI::RectType::FILL;

        SableUI::colour bgColour;

    private:
        /* private settings for rendering */
        SableUI::rect drawableRect = { 0, 0, 0, 0 };
        SableUI_Drawable::Rect bgDrawable;
    };

    class Listbox : public BaseElement
    {
    public:
        Listbox() : BaseElement() {};

        void AdditionalRender() override {};
    };
}