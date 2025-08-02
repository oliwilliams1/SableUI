#include "SableUI/element.h"
#include "SableUI/component.h"
#define SABLEUI_SUBSYSTEM "SableUI::Element"

/* child struct */
SableUI::Child::operator SableUI::Element* ()
{
    if (type == ChildType::ELEMENT)
        return element;
    else if (type == ChildType::COMPONENT)
        return component->GetRootElement();

    SableUI_Error("Unexpected union behaviour, you've been struck by the sun");
	return nullptr;
}

SableUI::Element::Element(Renderer* renderer, ElementType type)
{
    Init(renderer, type);
}

void SableUI::Element::Init(Renderer* renderer, ElementType type)
{
    this->renderer = renderer;
    this->type = type;

    switch (type)
    {
    case ElementType::RECT:
        drawable = new DrawableRect();
        break;

    case ElementType::IMAGE:
        drawable = new DrawableImage();
        break;

    case ElementType::TEXT:
        drawable = new DrawableText();
        break;

    case ElementType::DIV:
        drawable = nullptr;
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
            drRect->Update(rect, bgColour, 0.0f);
            renderer->Draw(drRect);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::IMAGE:
        if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
        {
            drImage->Update(rect);
            renderer->Draw(drImage);
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
            renderer->Draw(drText);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;

    case ElementType::DIV:
        break;

    default:
        SableUI_Error("Unknown ElementType");
        break;
    }
}

void SableUI::Element::SetInfo(const ElementInfo& info)
{
    this->width             = info.width;
    this->height            = info.height;
    this->marginTop         = info.marginTop;
    this->marginBottom      = info.marginBottom;
    this->marginLeft        = info.marginLeft;
    this->marginRight       = info.marginRight;
    this->paddingTop        = info.paddingTop;
    this->paddingBottom     = info.paddingBottom;
    this->paddingLeft       = info.paddingLeft;
    this->paddingRight      = info.paddingRight;
    this->centerX           = info.centerX;
    this->centerY           = info.centerY;
    this->wType             = info.wType;
    this->hType             = info.hType;
    this->bgColour          = info.bgColour;
    this->layoutDirection   = info.layoutDirection;
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
            renderer->Draw(drawable);

            LayoutChildren();
            for (Child child : children)
            {
                Element* childElement = (Element*)child;
                childElement->Render(z + 1);
            }
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
            renderer->Draw(drawable);
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
            renderer->Draw(drawable);
        }
        else
        {
            SableUI_Error("Dynamic cast failed");
        }
        break;
    }

    case ElementType::DIV:
    {
        LayoutChildren();
        for (Child child : children)
        {
            Element* childElement = (Element*)child;
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
    if (type == ElementType::IMAGE || type == ElementType::TEXT) SableUI_Error("Cannot add children to element of type image");
    
    children.push_back(child);
}

void SableUI::Element::AddChild(BaseComponent* component)
{
    children.push_back(component);
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

void SableUI::Element::SetText(const std::u32string& text, int fontSize, float lineHeight)
{
    if (type != ElementType::TEXT) SableUI_Error("Cannot set text on element not of type text");

    if (DrawableText* drText = dynamic_cast<DrawableText*>(drawable))
    {
        int reqHeight = drText->m_text.SetContent(text, drawable->m_rect.w, fontSize, lineHeight);
        height = reqHeight;
    }
    else
    {
        SableUI_Error("Dynamic cast failed");
    }
}

void SableUI::Element::LayoutChildren()
{
    if (type == ElementType::IMAGE || type == ElementType::TEXT)
    {
        SableUI_Warn("Should not be called on image or text element in the first place, here for dev purposes");
        return;
    }

    if (children.empty()) return;

    // calculate container area (no padding)
    int containerWidth = rect.w;
    int containerHeight = rect.h;

    if (containerWidth <= 0 || containerHeight <= 0)
    {
        SableUI_Warn("Container has zero or negative size");
        // set all children to zero size
        for (Child& child : children)
        {
            Element* childElement = (Element*)child;
            childElement->SetRect({ rect.x, rect.y, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    // calculate content area (after padding)
    int contentAreaX = rect.x + paddingLeft;
    int contentAreaY = rect.y + paddingTop;
    int contentAreaWidth = std::max(0, containerWidth - paddingLeft - paddingRight);
    int contentAreaHeight = std::max(0, containerHeight - paddingTop - paddingBottom);

    if (contentAreaWidth <= 0 || contentAreaHeight <= 0)
    {
        for (Child& child : children)
        {
            Element* childElement = (Element*)child;
            childElement->SetRect({ contentAreaX, contentAreaY, 0, 0 });
            childElement->LayoutChildren();
        }
        return;
    }

    bool isVerticalFlow = (layoutDirection == LayoutDirection::UP_DOWN || layoutDirection == LayoutDirection::DOWN_UP);
    bool isReverseFlow = (layoutDirection == LayoutDirection::DOWN_UP || layoutDirection == LayoutDirection::RIGHT_LEFT);

    int totalFixedMainAxis = 0;
    int totalMarginMainAxis = 0;
    int fillMainAxisCount = 0;

    for (Child& child : children)
    {
        Element* childElement = (Element*)child;

        if (isVerticalFlow)
        {
            totalMarginMainAxis += childElement->marginTop + childElement->marginBottom;

            if (childElement->hType == RectType::FIXED)
            {
                totalFixedMainAxis += childElement->height;
            }
            else if (childElement->hType == RectType::FILL)
            {
                fillMainAxisCount++;
            }
        }
        else
        {
            totalMarginMainAxis += childElement->marginLeft + childElement->marginRight;

            if (childElement->wType == RectType::FIXED)
            {
                totalFixedMainAxis += childElement->width;
            }
            else if (childElement->wType == RectType::FILL)
            {
                fillMainAxisCount++;
            }
        }
    }

    int availableMainAxis = isVerticalFlow ? contentAreaHeight : contentAreaWidth;
    int remainingMainAxis = availableMainAxis - totalFixedMainAxis - totalMarginMainAxis;
    int fillMainAxisSize = (fillMainAxisCount > 0) ? std::max(0, remainingMainAxis / fillMainAxisCount) : 0;
    int fillMainAxisRemainder = (fillMainAxisCount > 0) ? remainingMainAxis % fillMainAxisCount : 0;

    int cursorX, cursorY;

    if (isReverseFlow)
    {
        if (isVerticalFlow) // DOWN_UP
        {
            cursorX = contentAreaX;
            cursorY = contentAreaY + contentAreaHeight;
        }
        else // RIGHT_LEFT
        {
            cursorX = contentAreaX + contentAreaWidth;
            cursorY = contentAreaY;
        }
    }
    else
    {
        cursorX = contentAreaX;
        cursorY = contentAreaY;
    }

    int distributedRemainder = 0;

    for (Child& child : children)
    {
        Element* childElement = (Element*)child;

        int childMarginWidth = childElement->marginLeft + childElement->marginRight;
        int childMarginHeight = childElement->marginTop + childElement->marginBottom;

        int childContentWidth, childContentHeight;

        if (isVerticalFlow)
        {
            if (childElement->hType == RectType::FIXED)
            {
                childContentHeight = childElement->height;
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

            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
            }
            else
            {
                childContentWidth = std::max(0, contentAreaWidth - childMarginWidth);
            }
        }
        else
        {
            if (childElement->wType == RectType::FIXED)
            {
                childContentWidth = childElement->width;
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
            else
            {
                childContentHeight = std::max(0, contentAreaHeight - childMarginHeight);
            }
        }

        int childTotalWidth = childContentWidth + childElement->paddingLeft + childElement->paddingRight + childMarginWidth;
        int childTotalHeight = childContentHeight + childElement->paddingTop + childElement->paddingBottom + childMarginHeight;

        int childX, childY;

        if (isReverseFlow)
        {
            if (isVerticalFlow) // DOWN_UP
            {
                cursorY -= childTotalHeight;
                childX = cursorX + childElement->marginLeft;
                childY = cursorY + childElement->marginTop;
            }
            else // RIGHT_LEFT
            {
                cursorX -= childTotalWidth;
                childX = cursorX + childElement->marginLeft;
                childY = cursorY + childElement->marginTop;
            }
        }
        else
        {
            if (isVerticalFlow) // UP_DOWN
            {
                childX = cursorX + childElement->marginLeft;
                childY = cursorY + childElement->marginTop;
                cursorY += childTotalHeight;
            }
            else // LEFT_RIGHT
            {
                childX = cursorX + childElement->marginLeft;
                childY = cursorY + childElement->marginTop;
                cursorX += childTotalWidth;
            }
        }

        if (isVerticalFlow)
        {
            if (childElement->centerX)
            {
                int availableCrossSpace = contentAreaWidth - childTotalWidth;
                if (availableCrossSpace > 0)
                {
                    childX = contentAreaX + availableCrossSpace / 2 + childElement->marginLeft;
                }
            }
        }
        else
        {
            if (childElement->centerY)
            {
                int availableCrossSpace = contentAreaHeight - childTotalHeight;
                if (availableCrossSpace > 0)
                {
                    childY = contentAreaY + availableCrossSpace / 2 + childElement->marginTop;
                }
            }
        }

        int childRectX = childX + childElement->paddingLeft;
        int childRectY = childY + childElement->paddingTop;
        int childRectWidth = std::max(0, childContentWidth);
        int childRectHeight = std::max(0, childContentHeight);

        childRectX = std::max(contentAreaX, std::min(childRectX, contentAreaX + contentAreaWidth - childRectWidth));
        childRectY = std::max(contentAreaY, std::min(childRectY, contentAreaY + contentAreaHeight - childRectHeight));

        int maxChildWidth = std::max(0, (contentAreaX + contentAreaWidth) - childRectX);
        int maxChildHeight = std::max(0, (contentAreaY + contentAreaHeight) - childRectY);
        childRectWidth = std::min(childRectWidth, maxChildWidth);
        childRectHeight = std::min(childRectHeight, maxChildHeight);

        SableUI::Rect childRect = { childRectX, childRectY, childRectWidth, childRectHeight };
        childElement->SetRect(childRect);

        childElement->LayoutChildren();
    }
}

SableUI::Element::~Element()
{
    delete drawable;

    for (Child& child : children)
    {
        if (child.type == ChildType::ELEMENT)
        {
            delete child.element;
        }
        else if (child.type == ChildType::COMPONENT)
        {
            delete child.component;
        }
        else
        {
            SableUI_Error("Unexpected union behaviour");
        }
    }
}