#include <SableUI/core/scroll_context.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/component.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/styles/styles.h>
#include <SableUI/utils/utils.h>
#include <algorithm>

using namespace SableUI::Style;

bool SableUI::ScrollData::operator!=(const ScrollData& other) const
{
    return viewportSize.x != other.viewportSize.x ||
        viewportSize.y != other.viewportSize.y ||
        contentSize.x != other.contentSize.x ||
        contentSize.y != other.contentSize.y;
}

void SableUI::ScrollUpdateHandler_Phase1(BaseComponent* comp, ScrollContext& ctx, const UIEventContext& eventCtx)
{
    Element* viewportEl = comp->GetElementById(ctx.GetViewportID());
    if (!viewportEl) return;

    if (RectBoundingBox(viewportEl->rect, eventCtx.mousePos))
    {
        if (eventCtx.scrollDelta.y != 0)
        {
            vec2 newPos = ctx.scrollPos;
            newPos.y -= eventCtx.scrollDelta.y * 50.0f;

            if (ctx.prevScrollData.contentSize.y > ctx.prevScrollData.viewportSize.y)
            {
                float maxScrollY = ctx.prevScrollData.contentSize.y - ctx.prevScrollData.viewportSize.y;
                newPos.y = std::clamp(newPos.y, 0.0f, maxScrollY);
            }
            else
            {
                newPos.y = 0.0f;
            }

            if (newPos.y != ctx.scrollPos.y)
            {
                ctx.scrollPos = newPos;
                comp->MarkDirty();
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
            comp->MarkDirty();
        }

        if (ctx.isDragging)
        {
            if (eventCtx.mouseReleased.test(SABLE_MOUSE_BUTTON_LEFT))
            {
                ctx.isDragging = false;
                ctx.barHovered = false;
                comp->MarkDirty();
            }
            else
            {
                float viewportH = ctx.prevScrollData.viewportSize.y;
                float contentH = ctx.prevScrollData.contentSize.y;

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
                            comp->MarkDirty();
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
            comp->MarkDirty();
        }
    }
}

void SableUI::ScrollUpdateHandler_Phase2(BaseComponent* comp, ScrollContext& ctx)
{
    Element* viewportEl = comp->GetElementById(ctx.GetViewportID());
    Element* contentEl = comp->GetElementById(ctx.GetContentID());

    if (!viewportEl || !contentEl) return;

    ScrollData currentData;
    currentData.viewportSize = vec2(viewportEl->rect.w, viewportEl->rect.h);
    currentData.contentSize = vec2(contentEl->rect.w, contentEl->rect.h);

    bool changed = (ctx.scrollData != currentData);

    ctx.scrollData = currentData;

    vec2 newScrollPos = ctx.scrollPos;

    if (currentData.contentSize.y > currentData.viewportSize.y)
    {
        float maxScrollY = currentData.contentSize.y - currentData.viewportSize.y;
        newScrollPos.y = std::clamp(newScrollPos.y, 0.0f, maxScrollY);
    }
    else
    {
        newScrollPos.y = 0.0f;
    }

    if (newScrollPos.y != ctx.scrollPos.y)
    {
        ctx.scrollPos = newScrollPos;
        comp->MarkDirty();
    }
    else if (changed)
    {
        comp->MarkDirty();
    }

    ctx.prevScrollData = currentData;
}

SableUI::ScrollViewScope::ScrollViewScope(ScrollContext& context, const ElementInfo& info)
    : ctx(context)
{
    bgColour = info.appearance.bg;

    // viewport
    SableUI::StartDiv(PackStyles(id(ctx.GetViewportID()), w_fill, h_fill, left_right, overflow_hidden));

    // content
    ElementInfo contentInfo = info;
    PackStylesToInfo(contentInfo, id(ctx.GetContentID()), w_fill, h_fit, mt(-static_cast<int>(ctx.scrollPos.y)));
    SableUI::StartDiv(contentInfo);
}

SableUI::ScrollViewScope::~ScrollViewScope()
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
                Div(id(ctx.GetBarID()), w_fit, p(padding), h_fill, bg(bgColour))
                {
                    RectElement(w(6), h(static_cast<int>(thumbHeight)), mt(static_cast<int>(topMargin)), rounded(3), bg(149, 149, 149));
                }
            }
            else
            {
                Div(id(ctx.GetBarID()), w_fit, p(padding), h_fill, bg(bgColour))
                {
                    RectElement(w(2), m(2), h(static_cast<int>(thumbHeight)), mt(static_cast<int>(topMargin)), rounded(1), bg(128, 128, 128));
                }
            }
        }
    }

    SableUI::EndDiv();
}
