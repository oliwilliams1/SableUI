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
    };

    struct ScrollContext
    {
        vec2 scrollPos = { 0, 0 };
        ScrollData scrollData;

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

        if (!viewportEl || !contentEl) return;

        ScrollData tempData{};
        tempData.viewportSize = vec2(viewportEl->rect.w, viewportEl->rect.h);
        tempData.contentSize = vec2(contentEl->rect.w, contentEl->rect.h);
        ctx.scrollData = tempData;

        if (RectBoundingBox(viewportEl->rect, eventCtx.mousePos))
        {
            if (eventCtx.scrollDelta.y != 0)
            {
                vec2 newPos = ctx.scrollPos;
                newPos.y -= eventCtx.scrollDelta.y * 50.0f;

                if (contentEl->rect.h > viewportEl->rect.h)
                {
                    float maxScrollY = static_cast<float>(contentEl->rect.h - viewportEl->rect.h);
                    newPos.y = std::clamp(newPos.y, 0.0f, maxScrollY);
                }
                else
                {
                    newPos.y = 0.0f;
                }

                if (newPos.y != ctx.scrollPos.y)
                {
                    ctx.scrollPos = newPos;
                    needsRerender = true;
                }
            }
        }

        Element* barEl = comp->GetElementById(ctx.GetBarID());
        if (barEl)
        {
            bool hover = RectBoundingBox(barEl->rect, eventCtx.mousePos);
            if (hover != ctx.barHovered)
            {
                ctx.barHovered = hover;
                needsRerender = true;
            }

            if (ctx.isDragging)
            {
                if (eventCtx.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT))
                {
                    ctx.isDragging = false;
                    ctx.barHovered = false;
                    needsRerender = true;
                }
                else
                {
                    float viewportH = tempData.viewportSize.y;
                    float contentH = tempData.contentSize.y;

                    if (contentH > viewportH)
                    {
                        float padding = 4.0f * 2.0f;
                        float fac = viewportH / contentH;
                        float thumbHeight = fac * viewportH;
                        float trackScrollableRange = viewportH - thumbHeight - padding;
                        float contentScrollableRange = contentH - viewportH;

                        if (trackScrollableRange > 0)
                        {
                            float ratio = contentScrollableRange / trackScrollableRange;
                            float mouseDeltaY = static_cast<float>(eventCtx.mousePos.y - ctx.dragOrigPos.y);
                            float newScrollY = ctx.dragStartScrollY + (mouseDeltaY * ratio);

                            newScrollY = std::clamp(newScrollY, 0.0f, contentScrollableRange);

                            if (newScrollY != ctx.scrollPos.y)
                            {
                                ctx.scrollPos.y = newScrollY;
                                needsRerender = true;
                            }
                        }
                    }
                }
            }
            else if (hover && eventCtx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
            {
                ctx.isDragging = true;
                ctx.dragOrigPos = eventCtx.mousePos;
                ctx.dragStartScrollY = ctx.scrollPos.y;
                needsRerender = true;
            }
        }

        if (needsRerender)
            comp->needsRerender = true;
    }

    struct ScrollViewScope
    {
        ScrollContext& ctx;

        explicit ScrollViewScope(ScrollContext& context, ElementInfo info)
            : ctx(context)
        {
            SableUI::StartDiv(style(ID(ctx.GetViewportID()) w_fill h_fill left_right overflow_hidden));

            info.setID(ctx.GetContentID())
                .setWType(SableUI::RectType::Fill)
                .setHType(SableUI::RectType::FitContent)
                .setMarginTop(-static_cast<int>(ctx.scrollPos.y));

            SableUI::StartDiv(info);
        }

        ~ScrollViewScope()
        {
            SableUI::EndDiv();

            if (ctx.scrollData.contentSize.y > 0 && ctx.scrollData.viewportSize.y > 0)
            {
                float fac = ctx.scrollData.viewportSize.y / ctx.scrollData.contentSize.y;
                if (fac < 1.0f)
                {
                    float thumbHeight = fac * ctx.scrollData.viewportSize.y;
                    float maxScroll = ctx.scrollData.contentSize.y - ctx.scrollData.viewportSize.y;
                    float progress = (maxScroll > 0) ? (ctx.scrollPos.y / maxScroll) : 0.0f;

                    int padding = 4;
                    float trackRange = ctx.scrollData.viewportSize.y - thumbHeight - (padding * 2);
                    float topMargin = progress * trackRange;

                    if (ctx.barHovered)
                    {
                        Div(ID(ctx.GetBarID()) w_fit p(padding) h_fill bg(28, 28, 28) rounded(4))
                        {
                            Rect(w(6) h(static_cast<int>(thumbHeight)) mt(static_cast<int>(topMargin)) rounded(3) bg(149, 149, 149));
                        }
                    }
                    else
                    {
                        Div(ID(ctx.GetBarID()) w_fit p(padding) h_fill bg(32, 32, 32) rounded(4))
                        {
                            Rect(w(2) m(2) h(static_cast<int>(thumbHeight)) mt(static_cast<int>(topMargin)) rounded(1) bg(128, 128, 128));
                        }
                    }
                }
            }

            SableUI::EndDiv();
        }
    };
}

#define ScrollViewCtx(ctx, ...) if (SableUI::ScrollViewScope _sv_scope(ctx, style(__VA_ARGS__)); true)