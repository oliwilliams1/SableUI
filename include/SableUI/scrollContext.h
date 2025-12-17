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
    struct ScrollContext
    {
        vec2 scrollPos = { 0, 0 };
        vec2 viewportSize = { 0, 0 };
        vec2 contentSize = { 0, 0 };

        bool isDragging = false;
        ivec2 dragOrigPos = { 0, 0 };
        float dragStartScrollY = 0.0f;
        bool barHovered = false;

        std::string GetViewportID() const { return "VP_" + std::to_string((size_t)this); }
        std::string GetContentID() const { return "CNT_" + std::to_string((size_t)this); }
        std::string GetBarID() const { return "BAR_" + std::to_string((size_t)this); }
    };

    inline void ProcessScroll(BaseComponent* comp, ScrollContext& ctx, const UIEventContext& eventCtx)
    {
        bool needsRerender = false;

        Element* viewportEl = comp->GetElementById(ctx.GetViewportID());
        Element* contentEl = comp->GetElementById(ctx.GetContentID());
        Element* barEl = comp->GetElementById(ctx.GetBarID());

        if (viewportEl && contentEl)
        {
            ctx.viewportSize = vec2(viewportEl->rect.w, viewportEl->rect.h);
            ctx.contentSize = vec2(contentEl->rect.w, contentEl->rect.h);
        }

        if (viewportEl && contentEl && RectBoundingBox(viewportEl->rect, eventCtx.mousePos))
        {
            if (eventCtx.scrollDelta.y != 0)
            {
                vec2 newPos = ctx.scrollPos;
                newPos.y -= eventCtx.scrollDelta.y * 50.0f;

                if (contentEl->rect.h > viewportEl->rect.h)
                    newPos.y = std::clamp(newPos.y, 0.0f, static_cast<float>(contentEl->rect.h - viewportEl->rect.h));
                else
                    newPos.y = 0.0f;

                if (newPos.y != ctx.scrollPos.y)
                {
                    ctx.scrollPos = newPos;
                    needsRerender = true;
                }
            }
        }

        if (needsRerender)
            comp->needsRerender = true;
    }

    struct ScrollViewScope
    {
        ScrollContext& ctx;

        explicit ScrollViewScope(ScrollContext& context, ElementInfo& info)
            : ctx(context)
        {
            // viewport
            SableUI::StartDiv(style(ID(ctx.GetViewportID()) w_fill h_fill overflow_hidden));

            // content
            SableUI::StartDiv(info ID(ctx.GetContentID()) w_fill h_fit mt(-ctx.scrollPos.y));
        }

        ~ScrollViewScope()
        {
            SableUI::EndDiv();

            if (ctx.contentSize.y > 0 && ctx.viewportSize.y > 0)
            {
                float fac = ctx.viewportSize.y / ctx.contentSize.y;
                if (fac < 1.0f)
                {
                    float thumbHeight = fac * ctx.viewportSize.y;
                    float maxScroll = ctx.contentSize.y - ctx.viewportSize.y;
                    float progress = ctx.scrollPos.y / maxScroll;

                    int padding = 4;
                    float trackRange = ctx.viewportSize.y - thumbHeight - (padding * 2);
                    float topMargin = progress * trackRange;
                }
            }

            SableUI::EndDiv();
        }
    };
}

#define ScrollViewCtx(ctx, ...) if (SableUI::ScrollViewScope _sv_scope(ctx, style(h_fill w_fill __VA_ARGS__)); true)