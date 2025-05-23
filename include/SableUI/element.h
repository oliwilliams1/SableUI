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
        SableUI::RectType wType = SableUI::RectType::FILL;
        SableUI::RectType hType = SableUI::RectType::FILL;
    };

    class BaseElement
    {
    public:
        BaseElement(const std::string name) : name(name) {}

        void SetInfo(const ElementInfo& info);

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

        SableUI::colour bgColour = SableUI::colour(128, 128, 128);
        std::string name = "undefined element";

    private:
        /* private settings for rendering */
        SableUI::rect drawableRect = { 0, 0, 0, 0 };
        SableUI_Drawable::Rect bgDrawable;
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