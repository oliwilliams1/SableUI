#pragma once
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>
#include <algorithm>
#include <string>

namespace SableUI
{
    struct ScrollData
    {
        vec2 viewportSize = { 0, 0 };
        vec2 contentSize = { 0, 0 };

        bool operator!=(const ScrollData& other) const;
    };

    struct ScrollContext
    {
        vec2 scrollPos = { 0, 0 };
        ScrollData scrollData;
        ScrollData prevScrollData;

        bool isDragging = false;
        ivec2 dragOrigPos = { 0, 0 };
        float dragStartScrollY = 0.0f;
        bool barHovered = false;

        std::string GetViewportID() const { return "VP_" + std::to_string((size_t)this); }
        std::string GetContentID() const { return "CNT_" + std::to_string((size_t)this); }
        std::string GetBarID() const { return "BAR_" + std::to_string((size_t)this); }
    };

    // Called during OnUpdate, handling user input
    void ScrollUpdateHandler_Phase1(BaseComponent* comp, ScrollContext& ctx, const UIEventContext& eventCtx);

    // Called during OnUpdatePostLayout, handling Element Sizes
    void ScrollUpdateHandler_Phase2(BaseComponent* comp, ScrollContext& ctx);

    struct ScrollViewScope
    {
    public:
        ScrollContext& ctx;

        explicit ScrollViewScope(ScrollContext& context, ElementInfo info);
        ~ScrollViewScope();
    
    private:
        Colour bgColour;
    };
}

#define ScrollUpdateHandler(scrollCtx)                          \
    SableUI::ScrollUpdateHandler_Phase1(this, scrollCtx, ctx)

#define ScrollUpdatePostLayoutHandler(scrollCtx)                \
    SableUI::ScrollUpdateHandler_Phase2(this, scrollCtx)

#define ScrollViewCtx(ctx, ...) if (SableUI::ScrollViewScope _sv_scope(ctx, SableUI::PackStyles(__VA_ARGS__)); true)