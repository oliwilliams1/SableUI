#pragma once
#include "SableUI/window.h"

/* non-macro user api */
namespace SableUI
{
    void SetElementBuilderContext(Renderer* renderer, Element* rootElement);
    void SetContext(Window* window);
    Element* GetCurrentElement();
    
    SplitterPanel* StartSplitter(PanelType orientation);
    void EndSplitter();
    Panel* AddPanel();
    
    Element* StartDiv(const ElementInfo& info = {}, SableUI::BaseComponent* child = nullptr);
    void EndDiv();

    Element* AddRect(const ElementInfo& info = {});
    Element* AddImage(const std::string& path, const ElementInfo& info = {});
    Element* AddText(const std::string& text, const ElementInfo& info = {});
    Element* AddTextU32(const SableString& text, const ElementInfo& info = {});
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

    struct SplitterScope
    {
    public:
        explicit SplitterScope(const PanelType& type)
        {
            SableUI::StartSplitter(type);
        }
        ~SplitterScope()
        {
            SableUI::EndSplitter();
        }
        SplitterScope(const SplitterScope&) = delete;
        SplitterScope& operator=(const SplitterScope&) = delete;
        SplitterScope(SplitterScope&&) = default;
        SplitterScope& operator=(SplitterScope&&) = default;
    };
}

/* scoped RAII rect guard api */
#define Rect(...) AddRect(SableUI::ElementInfo{} __VA_ARGS__)
#define Div(...) if (SableUI::DivScope CONCAT(_div_guard_, __LINE__)(SableUI::ElementInfo{} __VA_ARGS__); true)
#define Image(path, ...) AddImage(path, SableUI::ElementInfo{} __VA_ARGS__)
#define Text(text, ...) AddText(text, SableUI::ElementInfo{} __VA_ARGS__)
#define TextU32(text, ...) AddTextU32(text, SableUI::ElementInfo{} __VA_ARGS__)

#define style(...) SableUI::ElementInfo{} __VA_ARGS__
#define Component(T, info, ...) AddComponent<T>(__VA_ARGS__)->BackendInitialiseChild(this, style(info))

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

#define rgb(r, g, b)    SableUI::Colour(r, g, b)

/* style modifiers */
#define id(value)       .setID(value)
#define bg(...)         .setBgColour(SableUI::Colour(__VA_ARGS__))
#define w(value)        .setWidth(value)
#define minW(value)     .setMinWidth(value)
#define maxW(value)     .setMaxWidth(value)
#define h(value)        .setHeight(value)
#define minH(value)     .setMinHeight(value)
#define maxH(value)     .setMaxHeight(value)
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

#define dir(value)      .setLayoutDirection(value)

#define useState(variableName, setterName, T, initialValue) \
    T variableName = initialValue; \
    SableUI::StateSetter<T> setterName = SableUI::StateSetter<T>([this](const T& val) { \
        if (variableName == val) return; \
        this->variableName = val; \
        this->needsRerender = true; \
    })

#define onHover(callback)           .setOnHover(callback)
#define onHoverExit(callback)       .setOnHoverExit(callback)
#define onClick(callback)           .setOnClick(callback)
#define onSecondaryClick(callback)  .setOnSecondaryClick(callback)

#define HSplitter()                 if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::HORIZONTAL); true)
#define VSplitter()                 if (SableUI::SplitterScope CONCAT(_div_guard_, __LINE__)(SableUI::PanelType::VERTICAL); true)

#define Panel()                     SableUI::AddPanel();
#define PanelWith(T, ...)           SableUI::AddPanel()->AttachComponent<T>(__VA_ARGS__);