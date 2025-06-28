#pragma once
#include <memory>
#include <unordered_map>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"

namespace SableUI
{
    class Renderer;

    enum class ElementType
    {
        UNDEF,
        RECT,
        IMAGE,
        TEXT
    };

    struct ElementInfo
    {
        std::string name;                        // Custom name, used for attaching, editing components without a saved reference
        Colour bgColour = Colour(128, 128, 128); // Background colour for solid rect elements
        float xOffset = 0;                       // Horizontal offset from left side
        float yOffset = 0;                       // Vertical offset from top
        float width = 0;                         // Width in px, only useful if wType == RectType::FIXED
        float height = 0;                        // Height in px, only useful if hType == RectType::FIXED
        float padding = 0;                       // Inner padding of elements, useful when adding children elements
        bool centerX = false;                    // Is centered horizontally?
        bool centerY = false;                    // Is centered vertically?
        RectType wType = RectType::FILL;         // Can be FILL or FIXED, use fill to enable automatic dynamic resizing, fixed for fixed-width elements
        RectType hType = RectType::FILL;
        ElementType type = ElementType::UNDEF;   // Can be UNDEF (for init error checks), RECT (for solid rectangle elements), IMAGE, or TEXT
    };

    class Element
    {
    public:
        Element(const std::string name, Renderer* renderer, ElementType type);
        ~Element();

        void SetInfo(const ElementInfo& info);

        /* Base render (background) */
        void Render(int z = 1);

        void SetRect(const Rect& rect);

        void UpdateChildren();
        void AddChild(Element* child);
        void SetImage(const std::string& path);
        void SetText(const std::u32string& text, int fontSize = 12);

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

        std::string name = "unnamed element";

    private:
        /* Private settings for rendering */
        Rect rect = { 0, 0, 0, 0 };
        DrawableBase* drawable;
        std::vector<Element*> children;
        Renderer* renderer = nullptr;
        ElementType type = ElementType::UNDEF;
    };
}