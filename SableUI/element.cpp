#include <SableUI/element.h>
#include <SableUI/component.h>
#include <SableUI/memory.h>
#include <string>
#include <SableUI/console.h>
#include <SableUI/drawable.h>
#include <SableUI/events.h>
#include <SableUI/renderer.h>
#include <SableUI/utils.h>
#include <algorithm>
#include <functional>

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

    SableUI_Error("Unexpected union behaviour, you've been struck by the sun");
	return nullptr;
}

SableUI::Element::Element()
{
    n_elements++;
}

SableUI::Element::Element(RendererBackend* renderer, ElementType type)
{
    n_elements++;
    Init(renderer, type);
}

void SableUI::Element::Init(RendererBackend* renderer, ElementType type)
{
    this->renderer = renderer;
    this->type = type;

    switch (type)
    {
    case ElementType::RECT:
        drawable = SB_new<DrawableRect>();
        break;

    case ElementType::IMAGE:
        drawable = SB_new<DrawableImage>();
        break;

    case ElementType::TEXT:
        drawable = SB_new<DrawableText>();
        break;

    case ElementType::DIV:
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

    switch (type)
    {
    case ElementType::RECT:
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            drRect->Update(rect, bgColour, borderRadius);
            renderer->AddToDrawStack(drRect);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::IMAGE:
        if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
        {
            drImage->Update(rect, borderRadius);
            renderer->AddToDrawStack(drImage);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::TEXT:
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            rect.h = drText->m_text.UpdateMaxWidth(rect.w);
            height = rect.h;
            drText->Update(rect);
            renderer->AddToDrawStack(drText);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::DIV:
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            drRect->Update(rect, bgColour, borderRadius);
            renderer->AddToDrawStack(drRect);
        }
        else
        {
			SableUI_Error("Dynamic cast failed");
        }
        break;

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::SetInfo(const ElementInfo& info)
{
    this->ID                        = info.id;
    this->width                     = info.width;
    this->minWidth                  = info.minWidth;
    this->maxWidth                  = info.maxWidth;
    this->height                    = info.height;
    this->minHeight                 = info.minHeight;
    this->maxHeight                 = info.maxHeight;
    this->marginTop                 = info.marginTop;
    this->marginBottom              = info.marginBottom;
    this->marginLeft                = info.marginLeft;
    this->marginRight               = info.marginRight;
    this->paddingTop                = info.paddingTop;
    this->paddingBottom             = info.paddingBottom;
    this->paddingLeft               = info.paddingLeft;
    this->paddingRight              = info.paddingRight;
    this->fontSize                  = info._fontSize;
    this->lineHeight                = info._lineHeight;
    this->centerX                   = info._centerX;
    this->centerY                   = info._centerY;
    this->borderRadius              = info._borderRadius;
    this->wType                     = info.wType;
    this->hType                     = info.hType;
    this->bgColour                  = info.bgColour;
    this->layoutDirection           = info.layoutDirection;
    this->textColour                = info.textColour;
    this->textJustification         = info.textJustification;
    this->m_onHoverFunc             = info.onHoverFunc;
    this->m_onHoverExitFunc         = info.onHoverExitFunc;
    this->m_onClickFunc             = info.onClickFunc;
    this->m_onSecondaryClickFunc    = info.onSecondaryClickFunc;
    this->m_onDoubleClickFunc       = info.onDoubleClickFunc;
}

void SableUI::Element::Render(int z)
{
    switch (type)
    {
    case ElementType::RECT:
    {
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            drawable->setZ(z);
            renderer->AddToDrawStack(drRect);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::IMAGE:
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

    case ElementType::TEXT:
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

    case ElementType::DIV:
    {
        if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
        {
            drawable->setZ(z);
            renderer->AddToDrawStack(drRect);
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
    if (type != ElementType::DIV) { SableUI_Error("Cannot add child to element not of type div"); return; };
    
    children.emplace_back(SB_new<Child>(child));
}

void SableUI::Element::AddChild(Child* child)
{
    if (type != ElementType::DIV) { SableUI_Error("Cannot add child to element not of type div"); return; };

    children.emplace_back(child);
}

void SableUI::Element::SetImage(const std::string& path)
{
    if (type != ElementType::IMAGE) SableUI_Error("Cannot set image on element not of type image");

    if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
    {
        drImage->m_texture.LoadTextureOptimised(path, width, height);
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

void SableUI::Element::SetText(const SableString& text)
{
    if (type != ElementType::TEXT) SableUI_Error("Cannot set text on element not of type text");

    if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
    {
        drText->m_text.m_colour = textColour;
        drText->m_text.SetContent(renderer, text, drawable->m_rect.w,
            fontSize, maxHeight, lineHeight, textJustification);
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

int SableUI::Element::GetMinWidth()
{
    int calculatedMinWidth = minWidth;

    if (type == ElementType::DIV)
    {
        bool isVerticalFlow = (layoutDirection == LayoutDirection::UP_DOWN || layoutDirection == LayoutDirection::DOWN_UP);
        if (isVerticalFlow)
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalWidth = childElement->GetMinWidth() + childElement->marginLeft + childElement->marginRight;
                calculatedMinWidth = std::max(calculatedMinWidth, childTotalWidth);
            }
        }
        else
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalWidth = childElement->GetMinWidth() + childElement->marginLeft + childElement->marginRight;
                calculatedMinWidth += childTotalWidth;
            }
        }
    }
    else if (type == ElementType::TEXT)
    {
        if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
        {
            calculatedMinWidth = std::max(calculatedMinWidth, drText->m_text.GetMinWidth());
        }
    }
    else
    {
        calculatedMinWidth = std::max(calculatedMinWidth, width);
    }

    return calculatedMinWidth + paddingLeft + paddingRight;
}

int SableUI::Element::GetMinHeight()
{
    int calculatedMinHeight = minHeight;

    if (type == ElementType::DIV)
    {
        bool isVerticalFlow = (layoutDirection == LayoutDirection::UP_DOWN || layoutDirection == LayoutDirection::DOWN_UP);
        if (isVerticalFlow)
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalHeight = childElement->GetMinHeight() + childElement->marginTop + childElement->marginBottom;
                calculatedMinHeight += childTotalHeight;
            }
        }
        else
        {
            for (Child* child : children)
            {
                Element* childElement = (Element*)*child;
                int childTotalHeight = childElement->GetMinHeight() + childElement->marginTop + childElement->marginBottom;
                calculatedMinHeight = std::max(calculatedMinHeight, childTotalHeight);
            }
        }
    }
    else if (type == ElementType::TEXT)
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
        calculatedMinHeight = std::max(calculatedMinHeight, height);
    }

    return calculatedMinHeight + paddingTop + paddingBottom;
}

void SableUI::Element::LayoutChildren()
{
    if (type != ElementType::DIV) return;
    if (children.empty()) return;

    rect.w = std::max(rect.w, GetMinWidth());
    rect.h = std::max(rect.h, GetMinHeight());

    // Calculate container area (no padding)
    ivec2 containerSize = { rect.w, rect.h };

    if (containerSize.x <= 0 || containerSize.y <= 0)
    {
        SableUI_Warn("Container has zero or negative size ID: \"%s\"", ((std::string)ID).c_str());
        // Set all children to zero size
        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->SetRect({ rect.x, rect.y, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    // Calculate content area (after padding)
    ivec2 contentAreaPosition = { rect.x + paddingLeft, rect.y + paddingTop };
    ivec2 contentAreaSize = { std::max(0, containerSize.x - paddingLeft - paddingRight),
                                     std::max(0, containerSize.y - paddingTop - paddingBottom) };

    if (contentAreaSize.x <= 0 || contentAreaSize.y <= 0)
    {
        for (Child* child : children)
        {
            Element* childElement = (Element*)*child;
            childElement->SetRect({ contentAreaPosition.x, contentAreaPosition.y, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    bool isVerticalFlow = (layoutDirection == LayoutDirection::UP_DOWN || layoutDirection == LayoutDirection::DOWN_UP);
    bool isReverseFlow = (layoutDirection == LayoutDirection::DOWN_UP || layoutDirection == LayoutDirection::RIGHT_LEFT);

    int totalFixedMainAxis = 0;
    int totalMarginMainAxis = 0;
    int totalPaddingOfFillElementsMainAxis = 0;
    int fillMainAxisCount = 0;

    for (Child* child : children)
    {
        Element* childElement = (Element*)*child;

        if (isVerticalFlow)
        {
            totalMarginMainAxis += childElement->marginTop + childElement->marginBottom;

            if (childElement->hType == RectType::FIXED)
            {
                totalFixedMainAxis += std::min(std::max(childElement->height, childElement->minHeight), childElement->maxHeight > 0 ? childElement->maxHeight : childElement->height);
            }
            else if (childElement->hType == RectType::FIT_CONTENT)
            {
                int minHeight = childElement->GetMinHeight();
                totalFixedMainAxis += std::min(std::max(0, minHeight), childElement->maxHeight > 0 ? childElement->maxHeight : minHeight);
            }
            else if (childElement->hType == RectType::FILL)
            {
                fillMainAxisCount++;
                totalPaddingOfFillElementsMainAxis += childElement->paddingTop + childElement->paddingBottom;
            }
        }
        else
        {
            totalMarginMainAxis += childElement->marginLeft + childElement->marginRight;

            if (childElement->wType == RectType::FIXED)
            {
                totalFixedMainAxis += std::min(std::max(childElement->width, childElement->minWidth), childElement->maxWidth > 0 ? childElement->maxWidth : childElement->width);
            }
            else if (childElement->wType == RectType::FIT_CONTENT)
            {
                int minWidth = childElement->GetMinWidth();
                totalFixedMainAxis += std::min(std::max(0, minWidth), childElement->maxWidth > 0 ? childElement->maxWidth : minWidth);
            }
            else if (childElement->wType == RectType::FILL)
            {
                fillMainAxisCount++;
                totalPaddingOfFillElementsMainAxis += childElement->paddingLeft + childElement->paddingRight;
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

        int childMarginWidth = childElement->marginLeft + childElement->marginRight;
        int childMarginHeight = childElement->marginTop + childElement->marginBottom;

        int childContentWidth, childContentHeight;

        if (isVerticalFlow)
        {
            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
            }
            else if (childElement->type == ElementType::TEXT || childElement->wType == RectType::FIT_CONTENT)
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }
            else
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }

            if (childElement->type == ElementType::TEXT)
            {
                if (DrawableText* drText = dynamic_cast<DrawableText*>(childElement->drawable))
                {
                    int newHeight = drText->m_text.UpdateMaxWidth(childContentWidth);
                    if (newHeight != childElement->height)
                    {
                        childElement->height = newHeight;
                    }
                    childContentHeight = newHeight;
                }
                else
                {
                    childContentHeight = childElement->height;
                }
            }
            else if (childElement->hType == RectType::FIXED)
            {
                childContentHeight = childElement->height;
            }
            else if (childElement->hType == RectType::FIT_CONTENT)
            {
                childContentHeight = std::max(0, childElement->GetMinHeight() - childElement->paddingTop - childElement->paddingBottom);
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
            childContentHeight = std::max(childContentHeight, childElement->minHeight);
            childContentHeight = (childElement->maxHeight > 0) ? std::min(childContentHeight, childElement->maxHeight) : childContentHeight;

            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
            }
            else if (childElement->wType == RectType::FIT_CONTENT)
            {
                childContentWidth = std::max(0, childElement->GetMinWidth() - childElement->paddingLeft - childElement->paddingRight);
            }
            else
            {
                childContentWidth = std::max(0, contentAreaSize.x - childMarginWidth);
            }

            childContentWidth = std::max(childContentWidth, childElement->minWidth);
            childContentWidth = (childElement->maxWidth > 0) ? std::min(childContentWidth, childElement->maxWidth) : childContentWidth;
        }
        else
        {
            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
            }
            else if (childElement->wType == RectType::FIT_CONTENT)
            {
                childContentWidth = std::max(0, childElement->GetMinWidth() - childElement->paddingLeft - childElement->paddingRight);
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

            if (childElement->hType == RectType::FIXED)
            {
                childContentHeight = childElement->height;
            }
            else if (childElement->hType == RectType::FIT_CONTENT)
            {
                childContentHeight = std::max(0, childElement->GetMinHeight() - childElement->paddingTop - childElement->paddingBottom);
            }
            else
            {
                childContentHeight = std::max(0, contentAreaSize.y - childMarginHeight);
            }
        }

        childContentWidth = std::max(0, childContentWidth);
        childContentHeight = std::max(0, childContentHeight);

        if (childElement->type == ElementType::TEXT && isVerticalFlow)
        {
            if (DrawableText* drText = dynamic_cast<DrawableText*>(childElement->drawable))
            {
                int newHeight = drText->m_text.UpdateMaxWidth(childContentWidth);
                if (newHeight != childElement->height)
                {
                    childElement->height = newHeight;
                }
                childContentHeight = newHeight;

            }
        }

        childElement->measuredHeight = childContentHeight;

        childContentWidth += childElement->paddingLeft + childElement->paddingRight;
        childContentHeight += childElement->paddingTop + childElement->paddingBottom;

        int childTotalWidth = childContentWidth + childMarginWidth;
        int childTotalHeight = childContentHeight + childMarginHeight;

        int childX, childY;

        if (isReverseFlow)
        {
            if (isVerticalFlow) // DOWN_UP
            {
                cursor.y -= childTotalHeight;
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
            }
            else // RIGHT_LEFT
            {
                cursor.x -= childTotalWidth;
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
            }
        }
        else
        {
            if (isVerticalFlow) // UP_DOWN
            {
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
                cursor.y += childTotalHeight;
            }
            else // LEFT_RIGHT
            {
                childX = cursor.x + childElement->marginLeft;
                childY = cursor.y + childElement->marginTop;
                cursor.x += childTotalWidth;
            }
        }

        if (isVerticalFlow)
        {
            if (childElement->centerX)
            {
                int available = contentAreaSize.x - childTotalWidth;
                if (available > 0)
                    childX = contentAreaPosition.x + available / 2 + childElement->marginLeft;
            }
        }
        else
        {
            if (childElement->centerY)
            {
                int available = contentAreaSize.y - childTotalHeight;
                if (available > 0)
                    childY = contentAreaPosition.y + available / 2 + childElement->marginTop;
            }
        }

        if (children.size() == 1)
        {
            if (childElement->centerX)
                childX = contentAreaPosition.x + (contentAreaSize.x - childTotalWidth) / 2 + childElement->marginLeft;
            if (childElement->centerY)
                childY = contentAreaPosition.y + (contentAreaSize.y - childTotalHeight) / 2 + childElement->marginTop;
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
}

SableUI::ElementInfo SableUI::Element::GetInfo() const
{
    ElementInfo info{};
    info.id                     = ID;
    info.bgColour               = bgColour;
    info.width                  = width;
    info.height                 = height;
    info.minWidth               = minWidth;
    info.maxWidth               = maxWidth;
    info.minHeight              = minHeight;
    info.maxHeight              = maxHeight;
    info.marginTop              = marginTop;
    info.marginBottom           = marginBottom;
    info.marginLeft             = marginLeft;
    info.marginRight            = marginRight;
    info.paddingTop             = paddingTop;
    info.paddingBottom          = paddingBottom;
    info.paddingLeft            = paddingLeft;
    info.paddingRight           = paddingRight;
    info._fontSize              = fontSize;
    info._lineHeight            = lineHeight;
    info._centerX               = centerX;
    info._centerY               = centerY;
    info._borderRadius          = borderRadius;
    info.wType                  = wType;
    info.hType                  = hType;
    info.type                   = type;
    info.layoutDirection        = layoutDirection;
    info.uniqueTextOrPath       = uniqueTextOrPath;
    info.textColour             = textColour;
    info.textJustification      = textJustification;

    return info;
}
static inline void hash_combine(std::size_t& seed, std::size_t v) {
    seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

#include "SableUI/SableUI.h"
static size_t ComputeHash(const SableUI::VirtualNode* vnode)
{
    size_t h = 1469598103934665603ULL;

    hash_combine(h, std::hash<int>()((int)vnode->type));

    if (vnode->uniqueTextOrPath.size() != 0)
    {
        std::string s = (std::string)(vnode->uniqueTextOrPath);
        hash_combine(h, std::hash<std::string>()(s));
    }

    hash_combine(h, std::hash<int>()((int)vnode->info.wType));
    hash_combine(h, std::hash<int>()((int)vnode->info.hType));

    hash_combine(h, std::hash<int>()(vnode->info.width));
    hash_combine(h, std::hash<int>()(vnode->info.height));
    hash_combine(h, std::hash<int>()(vnode->info.minWidth));
    hash_combine(h, std::hash<int>()(vnode->info.minHeight));
    hash_combine(h, std::hash<int>()(vnode->info.maxWidth));
    hash_combine(h, std::hash<int>()(vnode->info.maxHeight));

    hash_combine(h, std::hash<int>()(vnode->info.marginTop));
    hash_combine(h, std::hash<int>()(vnode->info.marginBottom));
    hash_combine(h, std::hash<int>()(vnode->info.marginLeft));
    hash_combine(h, std::hash<int>()(vnode->info.marginRight));
    hash_combine(h, std::hash<int>()(vnode->info.paddingTop));
    hash_combine(h, std::hash<int>()(vnode->info.paddingBottom));
    hash_combine(h, std::hash<int>()(vnode->info.paddingLeft));
    hash_combine(h, std::hash<int>()(vnode->info.paddingRight));
    hash_combine(h, std::hash<int>()(vnode->info._fontSize));
	hash_combine(h, std::hash<int>()(vnode->info._lineHeight));
    hash_combine(h, std::hash<int>()(vnode->info.textColour.r + vnode->info.textColour.g
                                    + vnode->info.textColour.b + vnode->info.textColour.a));

    hash_combine(h, std::hash<int>()((int)vnode->info.layoutDirection));
    hash_combine(h, std::hash<int>()((vnode->info.bgColour.r << 16) ^ (vnode->info.bgColour.g << 8) ^ vnode->info.bgColour.b));

    return h;
}

static size_t ComputeHash(const SableUI::Element* elem)
{
    size_t h = 1469598103934665603ULL;

    hash_combine(h, std::hash<int>()((int)elem->type));

    if (elem->uniqueTextOrPath.size() != 0)
    {
        std::string s = (std::string)(elem->uniqueTextOrPath);
        hash_combine(h, std::hash<std::string>()(s));
    }

    hash_combine(h, std::hash<int>()((int)elem->wType));
    hash_combine(h, std::hash<int>()((int)elem->hType));

    hash_combine(h, std::hash<int>()(elem->width));
    hash_combine(h, std::hash<int>()(elem->height));
    hash_combine(h, std::hash<int>()(elem->minWidth));
    hash_combine(h, std::hash<int>()(elem->minHeight));
    hash_combine(h, std::hash<int>()(elem->maxWidth));
    hash_combine(h, std::hash<int>()(elem->maxHeight));

    hash_combine(h, std::hash<int>()(elem->marginTop));
    hash_combine(h, std::hash<int>()(elem->marginBottom));
    hash_combine(h, std::hash<int>()(elem->marginLeft));
    hash_combine(h, std::hash<int>()(elem->marginRight));
    hash_combine(h, std::hash<int>()(elem->paddingTop));
    hash_combine(h, std::hash<int>()(elem->paddingBottom));
    hash_combine(h, std::hash<int>()(elem->paddingLeft));
    hash_combine(h, std::hash<int>()(elem->paddingRight));
    hash_combine(h, std::hash<int>()(elem->fontSize));
    hash_combine(h, std::hash<int>()(elem->lineHeight));
    hash_combine(h, std::hash<int>()(elem->textColour.r + elem->textColour.g
                                    + elem->textColour.b + elem->textColour.a));

    hash_combine(h, std::hash<int>()((int)elem->layoutDirection));
    hash_combine(h, std::hash<int>()((elem->bgColour.r << 16) ^ (elem->bgColour.g << 8) ^ elem->bgColour.b));

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
        Element* childEl = (Element*)*this->children[i];
        VirtualNode* childVn = vnode->children[i];

        std::size_t childElHash = ComputeHash(childEl);
        std::size_t childVnHash = ComputeHash(childVn);

        if (childElHash != childVnHash)
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

    switch (vnode->type)
    {
    case ElementType::DIV:
    {
        StartDiv(vnode->info, vnode->childComp);

        for (VirtualNode* child : vnode->children)
            BuildSingleElementFromVirtual(child);

        EndDiv();
        break;
    }

    case ElementType::RECT:
    {
        AddRect(vnode->info);
        break;
    }

    case ElementType::TEXT:
    {
        AddText((std::string)(vnode->uniqueTextOrPath), vnode->info);
        break;
    }

    case ElementType::TEXT_U32:
    {
        AddTextU32(vnode->uniqueTextOrPath, vnode->info);
        break;
    }

    case ElementType::IMAGE:
    {
        AddImage((std::string)(vnode->uniqueTextOrPath), vnode->info);
        break;
    }
    }
}

void SableUI::Element::el_PropagateEvents(const UIEventContext& ctx)
{
    if (RectBoundingBox(rect, ctx.mousePos))
    {
        if (!isHovered && m_onHoverFunc)
            m_onHoverFunc();

        isHovered = true;

        if (ctx.mousePressed[SABLE_MOUSE_BUTTON_LEFT] && m_onClickFunc) 
            m_onClickFunc();

        if (ctx.mousePressed[SABLE_MOUSE_BUTTON_RIGHT] && m_onSecondaryClickFunc)
            m_onSecondaryClickFunc();

        if (ctx.mouseDoubleClicked[SABLE_MOUSE_BUTTON_LEFT] && m_onDoubleClickFunc)
            m_onDoubleClickFunc();
    }
    else
    {
        if (isHovered && m_onHoverExitFunc)
			m_onHoverExitFunc();

        isHovered = false;
    }

    for (Child* child : children)
    {
        switch (child->type)
        {
        case ChildType::COMPONENT:
            child->component->comp_PropagateEvents(ctx);
			break;
        case ChildType::ELEMENT:
            child->element->el_PropagateEvents(ctx);
			break;
        default:
			SableUI_Error("Unexpected union behaviour, you've been struck by the sun");
            break;
        }
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