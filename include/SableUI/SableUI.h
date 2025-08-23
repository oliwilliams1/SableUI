#pragma once
#include "SableUI/window.h"

/* non-macro user api */
namespace SableUI
{
    void SetElementBuilderContext(Renderer* renderer, Element* rootElement);
    void SetContext(Window* window);
    
    SplitterPanel* StartSplitter(PanelType orientation);
    void EndSplitter();
    Panel* AddPanel();
    
    Element* StartDiv(const ElementInfo& info = {});
    void EndDiv();

    Element* AddRect(const ElementInfo& info = {});
    Element* AddImage(const std::string& path, const ElementInfo& info = {});
    Element* AddText(const std::string& text, const ElementInfo& info = {});
    Element* AddTextU32(const std::u32string& text, const ElementInfo& info = {});
}

namespace SableUI
{
    struct DivScope
    {
    public:
        explicit DivScope(const SableUI::ElementInfo& info)
        {
            SableUI::StartDiv(info);
        }
        ~DivScope()
        {
            SableUI::EndDiv();
        }
        DivScope(const DivScope&) = delete;
        DivScope& operator=(const DivScope&) = delete;
        DivScope(DivScope&&) = default;
        DivScope& operator=(DivScope&&) = default;
    };
}

/* scoped RAII rect guard api */
#define Rect(...) AddRect(SableUI::ElementInfo{} __VA_ARGS__);
#define Div(...) if (SableUI::DivScope CONCAT(_div_guard_, __LINE__)(SableUI::ElementInfo{} __VA_ARGS__); true)
#define Image(path, ...) AddImage(path, SableUI::ElementInfo{} __VA_ARGS__);
#define Text(text, ...) AddText(text, SableUI::ElementInfo{} __VA_ARGS__);
#define TextU32(text, ...) AddTextU32(text, SableUI::ElementInfo{} __VA_ARGS__);
#define UpdateStyle(element, ...) & element __VA_ARGS__

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

/*  Box Model
    /-----------------------------------------\
    | Margin (Top, Bottom, Left, Right)       |
    | |------------------------------------ | |
    | | Padding (Top, Bottom, Left, Right)  | |
    | | |-------------------------------- | | |
    | | | Content (rect)                  | | |
    | | |-------------------------------- | | |
    | |------------------------------------ | |
    \-----------------------------------------/ */

/* style modifiers */
#define id(value)       .setID(value)
#define bg(...)         .setBgColour(SableUI::Colour(__VA_ARGS__))
#define w(value)        .setWidth(value)
#define h(value)        .setHeight(value)
#define w_fill          .setWType(SableUI::RectType::FILL)
#define h_fill          .setHType(SableUI::RectType::FILL)
#define w_fixed         .setWType(SableUI::RectType::FIXED)
#define h_fixed         .setHType(SableUI::RectType::FIXED)
#define w_fit           .setWType(SableUI::RectType::FIT_CONTENT)
#define h_fit           .setHType(SableUI::RectType::FIT_CONTENT)

#define m(value)        .setMargin(value)
#define mx(value)       .setMarginX(value)
#define my(value)       .setMarginY(value)
#define mt(value)       .setMarginTop(value)
#define mb(value)       .setMarginBottom(value)
#define ml(value)       .setMarginLeft(value)
#define mr(value)       .setMarginRight(value)

#define p(value)        .setPaddingX(value).setPaddingY(value)
#define px(value)       .setPaddingX(value)
#define py(value)       .setPaddingY(value)
#define pt(value)       .setPaddingTop(value)
#define pb(value)       .setPaddingBottom(value)
#define pl(value)       .setPaddingLeft(value)
#define pr(value)       .setPaddingRight(value)

#define centerX         .setCenterX(true)
#define centerY         .setCenterY(true)
#define centerXY        .setCenterX(true).setCenterY(true)

#define left_right      .setLayoutDirection(SableUI::LayoutDirection::LEFT_RIGHT)
#define right_left      .setLayoutDirection(SableUI::LayoutDirection::RIGHT_LEFT)
#define up_down         .setLayoutDirection(SableUI::LayoutDirection::UP_DOWN)
#define down_up         .setLayoutDirection(SableUI::LayoutDirection::DOWN_UP)