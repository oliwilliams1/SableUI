#pragma once
#include "SableUI/window.h"


namespace SableUI
{
    void SetContext(Window* window);
    SplitterPanel* StartSplitter(PanelType orientation);
    void EndSplitter();

    Panel* AddPanel();

    void SetElementBuilderContext(Renderer* renderer, Element* rootElement);
    Element* StartDiv(const ElementInfo& info = {});
    void EndDiv();
    Element* AddText(const std::u32string& text, const ElementInfo& info = {});
    Element* StartRect(const ElementInfo& info = {});
    void EndRect();
}

class RectGuard
{
public:
    explicit RectGuard(const SableUI::ElementInfo& info)
    {
        SableUI::StartRect(info);
    }
    ~RectGuard()
    {
        SableUI::EndRect();
    }

    RectGuard(const RectGuard&) = delete;
    RectGuard& operator=(const RectGuard&) = delete;
    RectGuard(RectGuard&&) = default;
    RectGuard& operator=(RectGuard&&) = default;
};

#define Colour SableUI::Colour
#define Color Colour

/* scoped RAII rect guard api */
#define Rect(...) auto CONCAT(_rect_guard_, __LINE__) = RectGuard(SableUI::ElementInfo{} __VA_ARGS__)
#define RectContainer(...) { Rect(__VA_ARGS__);
#define EndContainer }

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define id(value)       .setID(value)
#define bg(...)         .setBgColour(Colour(__VA_ARGS__))
#define w(value)        .setWidth(value)
#define h(value)        .setHeight(value)
#define w_fill          .setWType(SableUI::RectType::FILL)
#define h_fill          .setHType(SableUI::RectType::FILL)
#define w_fixed         .setWType(SableUI::RectType::FIXED)
#define h_fixed         .setHType(SableUI::RectType::FIXED)

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

class SplitterGuard
{
public:
    explicit SplitterGuard(SableUI::PanelType orientation)
    {
        SableUI::StartSplitter(orientation);
    }
    ~SplitterGuard()
    {
        SableUI::EndSplitter();
    }

    SplitterGuard(const SplitterGuard&) = delete;
    SplitterGuard& operator=(const SplitterGuard&) = delete;
    SplitterGuard(SplitterGuard&&) = default;
    SplitterGuard& operator=(SplitterGuard&&) = default;
};