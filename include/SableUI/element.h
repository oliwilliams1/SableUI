#pragma once
#include <memory>
#include <unordered_map>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"
namespace SableUI
{
    class BaseElement
    {
    public:
        BaseElement(const std::string& name) : name(name), bgColour({ 255, 0, 0, 0 }) {};

        BaseElement(const std::string& name, SableUI::colour col) : name(name), bgColour(col) {};

        BaseElement(const std::string& name, float w, float h, 
            SableUI::colour col = SableUI::colour(128, 128, 128))
            : name(name), width(w), height(h), bgColour(col) {};

        BaseElement(const std::string& name, float xOffset, float yOffset, float w, float h, 
            SableUI::colour col = SableUI::colour(128, 128, 128))
            : name(name), xOffset(xOffset), yOffset(yOffset), width(w), height(h), bgColour(col) {};

        BaseElement(const std::string& name, float w, float h, 
            SableUI::RectType wType, SableUI::RectType hType, SableUI::colour col = SableUI::colour(128, 128, 128))
            : name(name), width(w), height(h), wType(wType), hType(hType), bgColour(col) {};

        BaseElement(const std::string& name, float xOffset, float yOffset, float w, float h, 
            SableUI::RectType wType, SableUI::RectType hType, SableUI::colour col = SableUI::colour(128, 128, 128))
            : name(name), xOffset(xOffset), yOffset(yOffset), width(w), height(h), wType(wType), hType(hType), bgColour(col) {};

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
        std::string name;

    private:
        /* private settings for rendering */
        SableUI::rect drawableRect = { 0, 0, 0, 0 };
        SableUI_Drawable::Rect bgDrawable;
    };
}