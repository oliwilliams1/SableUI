#include <SableUI/element.h>
#include <SableUI/component.h>
#include <SableUI/memory.h>
#include <SableUI/console.h>
#include <SableUI/drawable.h>
#include <SableUI/events.h>
#include <SableUI/renderer.h>
#include <SableUI/utils.h>
#include <string>
#include <algorithm>
#include <functional>
#include <SableUI/theme.h>
#include <type_traits>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "SableUI::Element"
using namespace  SableMemory;

static int n_elements = 0;
static int n_vElements = 0;

SableUI::Child::~Child()
{
    if (type == ChildType::ELEMENT)
    {
        SB_delete(element);
    }
}

/* child struct */
SableUI::Child::operator SableUI::Element*()
{
    if (type == ChildType::ELEMENT)
        return element;
    else if (type == ChildType::COMPONENT)
        return component->GetRootElement();

    SableUI_Runtime_Error("Unexpected union behaviour, you've been struck by the sun");
	return nullptr;
}

SableUI::Element::Element(RendererBackend* renderer, const ElementInfo& p_info)
{
    n_elements++;
    SetInfo(p_info);

    if (info.appearance.hasHoverBg)
        originalBg = info.appearance.bg;

    Init(renderer);
}

void SableUI::Element::Init(RendererBackend* renderer)
{
    this->renderer = renderer;

    if (drawable != nullptr) SableUI_Runtime_Error("Drawable already created, redundant Init() calls");

    switch (info.type)
    {
    case ElementType::Rect:
        drawable = SB_new<DrawableRect>();
        break;

    case ElementType::Image:
        drawable = SB_new<DrawableImage>();
        break;

    case ElementType::Text:
        drawable = SB_new<DrawableText>();
        break;

    case ElementType::Div:
        drawable = SB_new<DrawableRect>();
        break;

    default:
        SableUI_Runtime_Error("Unknown ElementType");
        drawable = nullptr;
        break;
    }
}

void SableUI::Element::SetRect(const Rect& r)
{
    this->rect = r;

    switch (info.type)
    {
    case ElementType::Rect:
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
            drRect->Update(rect, info.appearance.bg, info.appearance.radius, clipEnabled, clipRect);
        else
            SableUI_Error("Dynamic cast failed");
        break;

    case ElementType::Image:
        if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
            drImage->Update(rect, info.appearance.radius, clipEnabled, clipRect);
        else
            SableUI_Error("Dynamic cast failed");
        break;

    case ElementType::Text:
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            rect.h = drText->m_text.UpdateMaxWidth(rect.w);
            info.layout.height = rect.h;
            drText->Update(rect, clipEnabled, clipRect);
        }
        else
            SableUI_Error("Dynamic cast failed");
        break;

    case ElementType::Div:
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
            drRect->Update(rect, info.appearance.bg, info.appearance.radius, clipEnabled, clipRect);
        else
            SableUI_Error("Dynamic cast failed");
        break;

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::SetInfo(const ElementInfo& info)
{
    this->info = info;
}

void SableUI::Element::Render(int z)
{
    if (clipEnabled)
    {
        if (!rect.intersect(clipRect))
        {
            if (info.type == ElementType::Div)
            {
                for (Child* child : children)
                {
                    Element* childElement = (Element*)*child;
                    childElement->Render(z + 1);
                }
            }
            return;
        };
    }

    switch (info.type)
    {
    case ElementType::Rect:
    {
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            if (info.appearance.bg.a > 0)
            {
                drawable->setZ(z);
                renderer->AddToDrawStack(drRect);
            }
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::Image:
    {
        if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
        {
            drawable->setZ(z);
            renderer->AddToDrawStack(drImage);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::Text:
    {
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            drawable->setZ(z);
            renderer->AddToDrawStack(drText);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::Div:
    {
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            if (info.appearance.bg.a > 0)
            {
                drawable->setZ(z);
                renderer->AddToDrawStack(drRect);
            }
        }
        else
        {
			SableUI_Error("Dynamic cast failed");
        }

        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->Render(z + 1);
        }
        break;
    }

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::AddChild(Element* child)
{
    if (info.type != ElementType::Div) { SableUI_Error("Cannot add child to element not of type div"); return; };

    children.emplace_back(SB_new<Child>(child));
}

void SableUI::Element::AddChild(Child* child)
{
    if (info.type != ElementType::Div) { SableUI_Error("Cannot add child to element not of type div"); return; };

    children.emplace_back(child);
}

void SableUI::Element::SetImage(const std::string& path)
{
    if (info.type != ElementType::Image) SableUI_Error("Cannot set image on element not of type image");

    if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
    {
        drImage->m_texture.LoadTextureOptimised(path, info.layout.width, info.layout.height);
        info.text.content = path;
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

void SableUI::Element::SetText(const SableString& text)
{
    if (info.type != ElementType::Text) SableUI_Error("Cannot set text on element not of type text");

    if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
    {
        info.text.content = text;
        if (info.text.colour.has_value())
        {
            drText->m_text.m_colour = info.text.colour.value();
        }
        else
        {
            SableUI_Warn("Text colour not set, using default");
            drText->m_text.m_colour = GetTheme().text;
        }
        drText->m_text.SetContent(renderer, text, drawable->m_rect.w,
            info.text.fontSize, info.layout.maxH, info.text.lineHeight, info.text.justification);
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

int SableUI::Element::GetMinWidth()
{
    int calculatedMinWidth = info.layout.minW;

    if (info.appearance.clipChildren) return calculatedMinWidth + info.layout.pL + info.layout.pR;

    if (info.layout.wType == RectType::Fixed)
    {
        return std::max(calculatedMinWidth, info.layout.width) + info.layout.pL + info.layout.pR;
    }

    if (info.type == ElementType::Div)
    {
        bool isVerticalFlow = (info.layout.layoutDirection == LayoutDirection::UpDown
            || info.layout.layoutDirection == LayoutDirection::DownUp);
        if (isVerticalFlow)
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalWidth = childElement->GetMinWidth() + childElement->info.layout.mL + childElement->info.layout.mR;
                calculatedMinWidth = std::max(calculatedMinWidth, childTotalWidth);
            }
        }
        else
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalWidth = childElement->GetMinWidth() + childElement->info.layout.mL + childElement->info.layout.mR;
                calculatedMinWidth += childTotalWidth;
            }
        }
    }
    else if (info.type == ElementType::Text)
    {
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            calculatedMinWidth = std::max(calculatedMinWidth, drText->m_text.GetMinWidth(info.text.wrap));
        }
    }
    else
    {
        calculatedMinWidth = std::max(calculatedMinWidth, info.layout.width);
    }

    return calculatedMinWidth + info.layout.pL + info.layout.pR;
}

int SableUI::Element::GetMinHeight()
{
    int calculatedMinHeight = info.layout.minH;

    if (info.appearance.clipChildren) return calculatedMinHeight + info.layout.pT + info.layout.pB;

    if (info.layout.hType == RectType::Fixed)
    {
        return std::max(calculatedMinHeight, info.layout.height) + info.layout.pT + info.layout.pB;
    }

    if (info.type == ElementType::Div)
    {
        bool isVerticalFlow = (info.layout.layoutDirection == LayoutDirection::UpDown
            || info.layout.layoutDirection == LayoutDirection::DownUp);
        if (isVerticalFlow)
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalHeight = childElement->GetMinHeight() + childElement->info.layout.mT + childElement->info.layout.mB;
                calculatedMinHeight += childTotalHeight;
            }
        }
        else
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalHeight = childElement->GetMinHeight() + childElement->info.layout.mT + childElement->info.layout.mB;
                calculatedMinHeight = std::max(calculatedMinHeight, childTotalHeight);
            }
        }
    }
    else if (info.type == ElementType::Text)
    {
        if (measuredHeight > 0)
        {
            calculatedMinHeight = measuredHeight;
        }
        else
        {
            if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
            {
                calculatedMinHeight = std::max(calculatedMinHeight, drText->m_text.GetUnwrappedHeight());
            }
        }
    }
    else
    {
        calculatedMinHeight = std::max(calculatedMinHeight, info.layout.height);
    }

    return calculatedMinHeight + info.layout.pT + info.layout.pB;
}

void SableUI::Element::LayoutChildren()
{
    if (info.type != ElementType::Div) return;
    if (children.empty()) return;

    if (info.layout.pos.x != -1 || info.layout.pos.y != -1)
    {
        rect.x = info.layout.pos.x;
        rect.y = info.layout.pos.y;
    }

    int effectiveGapX = (info.layout.gapX != 0) ? info.layout.gapX : info.layout.gap;
    int effectiveGapY = (info.layout.gapY != 0) ? info.layout.gapY : info.layout.gap;

    bool isVerticalFlow = (info.layout.layoutDirection == LayoutDirection::UpDown
        || info.layout.layoutDirection == LayoutDirection::DownUp);
    bool isReverseFlow = (info.layout.layoutDirection == LayoutDirection::DownUp
        || info.layout.layoutDirection == LayoutDirection::RightLeft);

    size_t numChildren = children.size();
    if (numChildren > 1 && (effectiveGapX != 0 || effectiveGapY != 0))
    {
        for (size_t i = 0; i < numChildren - 1; i++) // Skip last child
        {
            Element* childElement = (Element*)*children[i];

            if (isVerticalFlow)
            {
                childElement->info.layout.mB += effectiveGapY;
            }
            else // horizontal flow
            {
                childElement->info.layout.mR += effectiveGapX;
            }
        }
    }

    rect.w = std::max(rect.w, GetMinWidth());
    rect.h = std::max(rect.h, GetMinHeight());

    // Calculate container area (no padding)
    ivec2 containerSize = { rect.w, rect.h };

    if (containerSize.x <= 0 || containerSize.y <= 0)
    {
        if (numChildren > 1 && (effectiveGapX != 0 || effectiveGapY != 0))
        {
            for (size_t i = 0; i < numChildren - 1; i++)
            {
                Element* childElement = (Element*)*children[i];
                if (isVerticalFlow)
                    childElement->info.layout.mB -= effectiveGapY;
                else
                    childElement->info.layout.mR -= effectiveGapX;
            }
        }

        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->SetRect({ rect.x, rect.y, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    // Calculate content area (after padding)
    ivec2 contentAreaPosition = { rect.x + info.layout.pL, rect.y + info.layout.pT };
    ivec2 contentAreaSize = { std::max(0, containerSize.x - info.layout.pL - info.layout.pR),
                              std::max(0, containerSize.y - info.layout.pT - info.layout.pB) };

    if (contentAreaSize.x <= 0 || contentAreaSize.y <= 0)
    {
        if (numChildren > 1 && (effectiveGapX != 0 || effectiveGapY != 0))
        {
            for (size_t i = 0; i < numChildren - 1; i++)
            {
                Element* childElement = (Element*)*children[i];
                if (isVerticalFlow)
                    childElement->info.layout.mB -= effectiveGapY;
                else
                    childElement->info.layout.mR -= effectiveGapX;
            }
        }

        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->SetRect({ contentAreaPosition.x, contentAreaPosition.y, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    int totalFixedMainAxis = 0;
    int totalMarginMainAxis = 0;
    int totalPaddingOfFillElementsMainAxis = 0;
    int fillMainAxisCount = 0;

    for (Child* child : children)
    {
        Element* childElement = (Element*)*child;

        Rect currentConstraint = { 0, 0, 0, 0 };
        bool hasConstraint = false;

        if (this->info.appearance.clipChildren)
        {
            currentConstraint = { rect.x, rect.y, rect.w, rect.h };
            hasConstraint = true;
        }

        if (this->clipEnabled)
        {
            if (hasConstraint)
            {
                currentConstraint = currentConstraint.getIntersection(this->clipRect);
            }
            else
            {
                currentConstraint = this->clipRect;
                hasConstraint = true;
            }
        }

        if (hasConstraint)
        {
            childElement->clipEnabled = true;
            childElement->clipRect = currentConstraint;
        }

        if (isVerticalFlow)
        {
            totalMarginMainAxis += childElement->info.layout.mT + childElement->info.layout.mB;

            if (childElement->info.layout.hType == RectType::Fixed)
            {
                totalFixedMainAxis += std::min(std::max(childElement->info.layout.height, childElement->info.layout.minH),
                    childElement->info.layout.maxH > 0 ? childElement->info.layout.maxH : childElement->info.layout.height);
            }
            else if (childElement->info.layout.hType == RectType::FitContent)
            {
                int minHeight = childElement->GetMinHeight();
                totalFixedMainAxis += std::min(std::max(0, minHeight),
                    childElement->info.layout.maxH > 0 ? childElement->info.layout.maxH : minHeight);
            }
            else if (childElement->info.layout.hType == RectType::Fill)
            {
                fillMainAxisCount++;
                totalPaddingOfFillElementsMainAxis += childElement->info.layout.pT + childElement->info.layout.pB;
            }
        }
        else
        {
            totalMarginMainAxis += childElement->info.layout.mL + childElement->info.layout.mR;

            if (childElement->info.layout.wType == RectType::Fixed)
            {
                totalFixedMainAxis += std::min(std::max(childElement->info.layout.width, childElement->info.layout.minW),
                    childElement->info.layout.maxW > 0 ? childElement->info.layout.maxW : childElement->info.layout.width);
            }
            else if (childElement->info.layout.wType == RectType::FitContent)
            {
                int minWidth = childElement->GetMinWidth();
                totalFixedMainAxis += std::min(std::max(0, minWidth),
                    childElement->info.layout.maxW > 0 ? childElement->info.layout.maxW : minWidth);
            }
            else if (childElement->info.layout.wType == RectType::Fill)
            {
                fillMainAxisCount++;
                totalPaddingOfFillElementsMainAxis += childElement->info.layout.pL + childElement->info.layout.pR;
            }
        }
    }

    int availableMainAxis = isVerticalFlow ? contentAreaSize.y : contentAreaSize.x;
    int remainingMainAxis = availableMainAxis - totalFixedMainAxis - totalMarginMainAxis - totalPaddingOfFillElementsMainAxis;
    int fillMainAxisSize = (fillMainAxisCount > 0) ? std::max(0, remainingMainAxis / fillMainAxisCount) : 0;
    int fillMainAxisRemainder = (fillMainAxisCount > 0) ? remainingMainAxis % fillMainAxisCount : 0;

    ivec2 cursor = isReverseFlow ?
        (isVerticalFlow ? ivec2(contentAreaPosition.x, contentAreaPosition.y + contentAreaSize.y) :
            ivec2(contentAreaPosition.x + contentAreaSize.x, contentAreaPosition.y)) :
        contentAreaPosition;

    int distributedRemainder = 0;

    for (Child* child : children)
    {
        Element* childElement = (Element*)*child;

        int childMarginWidth = childElement->info.layout.mL + childElement->info.layout.mR;
        int childMarginHeight = childElement->info.layout.mT + childElement->info.layout.mB;

        int childContentWidth, childContentHeight;

        if (isVerticalFlow)
        {
            if (childElement->info.layout.wType == RectType::Fixed)
            {
                childContentWidth = childElement->info.layout.width;
            }
            else if (childElement->info.type == ElementType::Text
                || childElement->info.layout.wType == RectType::FitContent)
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }
            else
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }

            if (childElement->info.type == ElementType::Text)
            {
                if (DrawableText* drText = dynamic_cast<DrawableText*>(childElement->drawable))
                {
                    int newHeight = drText->m_text.UpdateMaxWidth(childContentWidth);
                    if (newHeight != childElement->info.layout.height)
                    {
                        childElement->info.layout.height = newHeight;
                    }
                    childContentHeight = newHeight;
                }
                else
                {
                    childContentHeight = childElement->info.layout.height;
                }
            }
            else if (childElement->info.layout.hType == RectType::Fixed)
            {
                childContentHeight = childElement->info.layout.height;
            }
            else if (childElement->info.layout.hType == RectType::FitContent)
            {
                childContentHeight = std::max(0, childElement->GetMinHeight() - childElement->info.layout.pT - childElement->info.layout.pB);
            }
            else
            {
                childContentHeight = fillMainAxisSize;
                if (distributedRemainder < fillMainAxisRemainder)
                {
                    childContentHeight += 1;
                    distributedRemainder++;
                }
            }
            childContentHeight = std::max(childContentHeight, childElement->info.layout.minH);
            childContentHeight = (childElement->info.layout.maxH > 0) ?
                std::min(childContentHeight, childElement->info.layout.maxH) : childContentHeight;

            if (childElement->info.layout.wType == RectType::Fixed)
            {
                childContentWidth = childElement->info.layout.width;
            }
            else if (childElement->info.layout.wType == RectType::FitContent)
            {
                childContentWidth = std::max(0, childElement->GetMinWidth() - childElement->info.layout.pL - childElement->info.layout.pR);
            }
            else
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }

            childContentWidth = std::max(childContentWidth, childElement->info.layout.minW);
            childContentWidth = (childElement->info.layout.maxW > 0) ?
                std::min(childContentWidth, childElement->info.layout.maxW) : childContentWidth;
        }
        else
        {
            if (childElement->info.layout.wType == RectType::Fixed)
            {
                childContentWidth = childElement->info.layout.width;
            }
            else if (childElement->info.layout.wType == RectType::FitContent)
            {
                childContentWidth = std::max(0, childElement->GetMinWidth() - childElement->info.layout.pL - childElement->info.layout.pR);
            }
            else
            {
                childContentWidth = fillMainAxisSize;
                if (distributedRemainder < fillMainAxisRemainder)
                {
                    childContentWidth += 1;
                    distributedRemainder++;
                }
            }

            if (childElement->info.layout.hType == RectType::Fixed)
            {
                childContentHeight = childElement->info.layout.height;
            }
            else if (childElement->info.layout.hType == RectType::FitContent)
            {
                childContentHeight = std::max(0, childElement->GetMinHeight() - childElement->info.layout.pT - childElement->info.layout.pB);
            }
            else
            {
                childContentHeight = std::max(0, contentAreaSize.y - childMarginHeight);
            }
        }

        childContentWidth = std::max(0, childContentWidth);
        childContentHeight = std::max(0, childContentHeight);

        if (childElement->info.type == ElementType::Text && isVerticalFlow)
        {
            if (DrawableText* drText = dynamic_cast<DrawableText*>(childElement->drawable))
            {
                int newHeight = drText->m_text.UpdateMaxWidth(childContentWidth);
                if (newHeight != childElement->info.layout.height)
                {
                    childElement->info.layout.height = newHeight;
                }
                childContentHeight = newHeight;
            }
        }

        childElement->measuredHeight = childContentHeight;

        childContentWidth += childElement->info.layout.pL + childElement->info.layout.pR;
        childContentHeight += childElement->info.layout.pT + childElement->info.layout.pB;

        int childTotalWidth = childContentWidth + childMarginWidth;
        int childTotalHeight = childContentHeight + childMarginHeight;

        int childX, childY;

        if (isReverseFlow)
        {
            if (isVerticalFlow) // DownUp
            {
                cursor.y -= childTotalHeight;
                childX = cursor.x + childElement->info.layout.mL;
                childY = cursor.y + childElement->info.layout.mT;
            }
            else // RightLeft
            {
                cursor.x -= childTotalWidth;
                childX = cursor.x + childElement->info.layout.mL;
                childY = cursor.y + childElement->info.layout.mT;
            }
        }
        else
        {
            if (isVerticalFlow) // UpDown
            {
                childX = cursor.x + childElement->info.layout.mL;
                childY = cursor.y + childElement->info.layout.mT;
                cursor.y += childTotalHeight;
            }
            else // LeftRight
            {
                childX = cursor.x + childElement->info.layout.mL;
                childY = cursor.y + childElement->info.layout.mT;
                cursor.x += childTotalWidth;
            }
        }

        if (isVerticalFlow)
        {
            if (childElement->info.layout.centerX)
            {
                int available = contentAreaSize.x - childTotalWidth;
                if (available > 0)
                    childX = contentAreaPosition.x + available / 2 + childElement->info.layout.mL;
            }
        }
        else
        {
            if (childElement->info.layout.centerY)
            {
                int available = contentAreaSize.y - childTotalHeight;
                if (available > 0)
                    childY = contentAreaPosition.y + available / 2 + childElement->info.layout.mT;
            }
        }

        if (children.size() == 1)
        {
            if (childElement->info.layout.centerX)
                childX = contentAreaPosition.x + (contentAreaSize.x - childTotalWidth) / 2 + childElement->info.layout.mL;
            if (childElement->info.layout.centerY)
                childY = contentAreaPosition.y + (contentAreaSize.y - childTotalHeight) / 2 + childElement->info.layout.mT;
        }

        Rect childFinalRect = {
            childX,
            childY,
            std::min(childContentWidth, (contentAreaPosition.x + contentAreaSize.x) - childX),
            std::min(childContentHeight, (contentAreaPosition.y + contentAreaSize.y) - childY)
        };

        childElement->SetRect(childFinalRect);
        childElement->LayoutChildren();
    }

    if (numChildren > 1 && (effectiveGapX != 0 || effectiveGapY != 0))
    {
        for (size_t i = 0; i < numChildren - 1; i++)
        {
            Element* childElement = (Element*)*children[i];

            if (isVerticalFlow)
            {
                childElement->info.layout.mB -= effectiveGapY;
            }
            else
            {
                childElement->info.layout.mR -= effectiveGapX;
            }
        }
    }
}

void SableUI::Element::RegisterForHover()
{
    if (info.appearance.hasHoverBg && m_owner)
        m_owner->RegisterHoverElement(this);
}

SableUI::ElementInfo SableUI::Element::GetInfo() const
{
    return info;
}

static inline void hash_combine(std::size_t& seed, std::size_t v) {
    seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

#include <SableUI/SableUI.h>
static size_t ComputeHash(const SableUI::ElementInfo& info)
{
    size_t h = 1469598103934665603ULL;

    hash_combine(h, std::hash<int>()((int)info.type));
    if (info.text.content.size() != 0)
    {
        std::string s = (std::string)(info.text.content);
        hash_combine(h, std::hash<std::string>()(s));
    }

    hash_combine(h, ((int)info.layout.wType << 8) | (int)info.layout.hType);

    hash_combine(h, (info.layout.width << 16) | info.layout.height);
    hash_combine(h, (info.layout.minW << 16) | info.layout.minH);
    hash_combine(h, (info.layout.maxW << 16) | info.layout.maxH);

    hash_combine(h, (info.layout.mT << 24) | (info.layout.mR << 16) |
        (info.layout.mB << 8) | info.layout.mL);

    hash_combine(h, (info.layout.pT << 24) | (info.layout.pR << 16) |
        (info.layout.pB << 8) | info.layout.pL);

    hash_combine(h, (info.text.fontSize << 16) |
        (static_cast<int>(info.text.lineHeight * 1000) & 0xFFFF));

    if (info.text.colour.has_value())
    {
        const SableUI::Colour& c = info.text.colour.value();
		hash_combine(h, (c.r << 24) | (c.g << 16) |
			(c.b << 8) | c.a);
    }

    hash_combine(h, ((int)info.text.justification << 16) |
        (info.text.wrap ? 1 : 0));

    hash_combine(h, ((int)info.layout.layoutDirection << 24) |
        (info.layout.centerX ? (1 << 16) : 0) |
        (info.layout.centerY ? (1 << 8) : 0) |
        (static_cast<int>(info.appearance.radius) & 0xFF));

    hash_combine(h, (info.appearance.bg.r << 24) | (info.appearance.bg.g << 16) |
        (info.appearance.bg.b << 8) | info.appearance.bg.a);

    return h;
}

bool SableUI::Element::Reconcile(VirtualNode* vnode)
{
    if (this->children.size() != vnode->children.size())
    {
        for (Child* child : this->children)
            SB_delete(child);
        this->children.clear();

        SetElementBuilderContext(this->renderer, this, false);
        BuildRealSubtreeFromVirtual(vnode);

        return true;
    }

    for (size_t i = 0; i < this->children.size(); i++)
    {
        Child* childWrapper = this->children[i];

        Element* childEl = (Element*)*childWrapper;
        VirtualNode* childVn = vnode->children[i];

        std::size_t childElHash = ComputeHash(childEl->info);
        std::size_t childVnHash = ComputeHash(childVn->info);

        bool componentMismatch = false;
        bool isComponent = (childWrapper->type == ChildType::COMPONENT);
        bool wantsComponent = (childVn->childComp != nullptr);

        if (isComponent != wantsComponent)
        {
            componentMismatch = true;
        }
        else if (isComponent)
        {
            if (childWrapper->component != childVn->childComp)
            {
                componentMismatch = true;
            }
        }

        if (childElHash != childVnHash || componentMismatch)
        {
            for (Child* child : this->children)
                SB_delete(child);
            this->children.clear();

            SetElementBuilderContext(this->renderer, this, false);
            BuildRealSubtreeFromVirtual(vnode);

            return true;
        }
    }

    bool anyChildChanged = false;
    for (size_t i = 0; i < this->children.size(); i++)
    {
        Element* childEl = (Element*)*this->children[i];
        VirtualNode* childVn = vnode->children[i];

        bool childChanged = childEl->Reconcile(childVn);
        if (childChanged)
            anyChildChanged = true;
    }

    return anyChildChanged;
}

void SableUI::Element::BuildRealSubtreeFromVirtual(VirtualNode* vnode)
{
    if (!vnode) return;

    for (VirtualNode* vchild : vnode->children)
    {
        BuildSingleElementFromVirtual(vchild);
    }
}

void SableUI::Element::BuildSingleElementFromVirtual(VirtualNode* vnode)
{
    if (!vnode) return;

    switch (vnode->info.type)
    {
    case ElementType::Div:
    {
        StartDiv(vnode->info, vnode->childComp);

        for (VirtualNode* child : vnode->children)
            BuildSingleElementFromVirtual(child);

        EndDiv();
        break;
    }

    case ElementType::Rect:
    {
        AddRect(vnode->info);
        break;
    }

    case ElementType::Text:
    {
        AddText(vnode->info.text.content, vnode->info);
        break;
    }

    case ElementType::Image:
    {
        AddImage(vnode->info.text.content, vnode->info);
        break;
    }
    
	default:
		break;
    }
}

void SableUI::Element::el_PropagateEvents(const UIEventContext& ctx)
{
    if (RectBoundingBox(rect, ctx.mousePos))
    {
        if (ctx.mouseReleased[SABLE_MOUSE_BUTTON_LEFT] && info.onClickFunc)
            info.onClickFunc();

        if (ctx.mouseReleased[SABLE_MOUSE_BUTTON_RIGHT] && info.onSecondaryClickFunc)
            info.onSecondaryClickFunc();

        if (ctx.mouseDoubleClicked[SABLE_MOUSE_BUTTON_LEFT] && info.onDoubleClickFunc)
            info.onDoubleClickFunc();
    }

    for (Child* child : children)
    {
        if (child->type == ChildType::COMPONENT)
            child->component->comp_PropagateEvents(ctx);
        else
            child->element->el_PropagateEvents(ctx);
    }
}

bool SableUI::Element::el_PropagateComponentStateChanges(bool* hasContentsChanged)
{
    bool res = false;
    for (Child* child : children)
    {
        switch (child->type)
        {
        case ChildType::COMPONENT:
			res = res || child->component->comp_PropagateComponentStateChanges(hasContentsChanged);
            break;
        case ChildType::ELEMENT:
			res = res || child->element->el_PropagateComponentStateChanges(hasContentsChanged);
            break;
        default:
			SableUI_Error("Unexpected union behaviour, you've been struck by the sun");
        }

        if (res) return true;
	}

    return res;
}

SableUI::Element* SableUI::Element::GetElementById(const SableString& id)
{
    if (this->info.id == id)
        return this;

    for (Child* child : children)
    {
        Element* childElement = (Element*)*child;
        if (!childElement) continue;

        Element* found = childElement->GetElementById(id);
        if (found)
            return found;
    }

    return nullptr;
}

SableUI::Element::~Element()
{
    n_elements--;
    renderer->ClearDrawable(drawable);
    SB_delete(drawable);

    for (Child* child : children) SB_delete(child);
    children.clear();
}

SableUI::VirtualNode::VirtualNode()
{
    n_vElements++;
}

SableUI::VirtualNode::~VirtualNode()
{
    n_vElements--;
    for (VirtualNode* child : children) SB_delete(child);
	children.clear();
}

int SableUI::Element::GetNumInstances()
{
    return n_elements;
}

int SableUI::VirtualNode::GetNumInstances()
{
	return n_vElements;
}