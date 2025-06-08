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
        ElementType type = ElementType::UNDEF;
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
        void SetText(const std::u32string& text);

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