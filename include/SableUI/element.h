#pragma once
#include <memory>
#include <unordered_map>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"
namespace SableUI
{
    struct ElementInfo
    {
        std::string name;
        SableUI::colour bgColour = SableUI::colour(128, 128, 128);
        float xOffset = 0;
        float yOffset = 0;
        float width = 0;
        float height = 0;
        float padding = 0;
        bool centerX = false;
        bool centerY = false;
        SableUI::RectType wType = SableUI::RectType::FILL;
        SableUI::RectType hType = SableUI::RectType::FILL;
    };

    class BaseElement
    {
    public:
        BaseElement(const std::string name) : name(name) {}

        void SetInfo(const ElementInfo& info);

        /* Base render (background) */
        void Render(int z = 1);

        /* Virtual render to add additional, custom elements */
        virtual void AdditionalRender() {};

        void SetRect(const SableUI::rect& rect);

        void UpdateChildren();
        void AddChild(BaseElement* child);

        /* user-level settings for rect */
        float xOffset = 0;
        float yOffset = 0;
        float width = 0;
        float height = 0;
        float padding = 0;
        bool centerX = false;
        bool centerY = false;

        SableUI::RectType wType = SableUI::RectType::FILL;
        SableUI::RectType hType = SableUI::RectType::FILL;

        SableUI::colour bgColour = SableUI::colour(128, 128, 128);
        std::string name = "undefined element";

    private:
        /* private settings for rendering */
        SableUI::rect drawableRect = { 0, 0, 0, 0 };
        SableUI::DrawableRect bgDrawable;
        std::vector<BaseElement*> children;
    };

    /* Central element manager */
    class ElementArena
    {
    public:
        static BaseElement* CreateElement(const std::string& name);
        static BaseElement* GetElement(const std::string& name);
        static void ShutdownArena();

    private:
        static std::vector<BaseElement*> elements;
    };
}