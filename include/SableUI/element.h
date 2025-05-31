#pragma once
#include <memory>
#include <unordered_map>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"

namespace SableUI
{
    class Renderer;

    struct ElementInfo
    {
        std::string name;
        Colour bgColour = Colour(128, 128, 128);
        float xOffset = 0;
        float yOffset = 0;
        float width = 0;
        float height = 0;
        float padding = 0;
        bool centerX = false;
        bool centerY = false;
        RectType wType = RectType::FILL;
        RectType hType = RectType::FILL;
    };

    class BaseElement
    {
    public:
        BaseElement(const std::string name, Renderer* renderer)
            : name(name), renderer(renderer) {}

        void SetInfo(const ElementInfo& info);

        /* Base render (background) */
        void Render(int z = 1);

        /* Virtual render to add additional, custom elements */
        virtual void AdditionalRender() {};

        void SetRect(const rect& rect);

        void UpdateChildren();
        void AddChild(BaseElement* child);

        /* User-level settings for rect */
        float xOffset = 0;
        float yOffset = 0;
        float width = 0;
        float height = 0;
        float padding = 0;
        bool centerX = false;
        bool centerY = false;

        RectType wType = RectType::FILL;
        RectType hType = RectType::FILL;

        Colour bgColour = Colour(128, 128, 128);
        std::string name = "undefined element";

    private:
        /* Private settings for rendering */
        rect drawableRect = { 0, 0, 0, 0 };
        DrawableRect bgDrawable;
        std::vector<BaseElement*> children;
        Renderer* renderer = nullptr;
    };
}