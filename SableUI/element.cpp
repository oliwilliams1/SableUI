#include "SableUI/element.h"

using namespace SableUI;

SableUI::Element::Element(const char* name, Renderer* renderer, ElementType type)
{
    Init(name, renderer, type);
}

void SableUI::Element::Init(const char* name, Renderer* renderer, ElementType type)
{
    this->name = name;
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
    this->name = info.name;
    this->xOffset = info.xOffset;
    this->yOffset = info.yOffset;
    this->width = info.width;
    this->height = info.height;
    this->padding = info.padding;
    this->wType = info.wType;
    this->hType = info.hType;
    this->centerX = info.centerX;
    this->centerY = info.centerY;
    this->bgColour = info.bgColour;
    this->type = info.type;
    this->layoutDirection = info.layoutDirection;
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

            UpdateChildren();
            for (Element* element : children)
            {
                element->Render(z + 1);
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

    case ElementType::DIV:
        UpdateChildren();
        for (Element* element : children)
        {
            element->Render(z + 1);
        }
        break;

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

void SableUI::Element::UpdateChildren()
{
    if (type == ElementType::IMAGE || type == ElementType::TEXT) return;

    ivec2 cursor = { rect.x + padding, rect.y + padding };
    ivec2 bounds = { rect.x + rect.w - padding, rect.y + rect.h - padding };

    if (wType == RectType::FIT_CONTENT)
    {
        width = GetWidth();
    }
    if (hType == RectType::FIT_CONTENT)
    {
        height = GetHeight();
    }

    int totalSize = 0;
    int fillCount = 0;

    for (Element* child : children)
    {
        if (layoutDirection == LayoutDirection::VERTICAL)
        {
            if (child->hType == RectType::FIXED || child->hType == RectType::FIT_CONTENT)
            {
                totalSize += child->height + child->yOffset;
            }
            else if (child->hType == RectType::FILL)
            {
                fillCount++;
            }
        }
        else
        {
            if (child->wType == RectType::FIXED || child->wType == RectType::FIT_CONTENT)
            {
                totalSize += child->width + child->xOffset;
            }
            else if (child->wType == RectType::FILL)
            {
                fillCount++;
            }
        }
    }

    int availableSize = (layoutDirection == LayoutDirection::VERTICAL) ?
        (rect.h - 2 * padding) : (rect.w - 2 * padding);
    int fillSizePerElement = 0;

    if (fillCount > 0)
    {
        fillSizePerElement = (availableSize - totalSize) / fillCount;
    }
    else
    {
        fillSizePerElement = 0;
    }

    int remainingSize = (fillCount > 0) ? (availableSize - totalSize) % fillCount : 0;

    for (Element* child : children)
    {
        SableUI::Rect childRect = { 0, 0, 0, 0 };

        int availableWidth = rect.w - 2 * padding;
        int availableHeight = rect.h - 2 * padding;

        if (layoutDirection == LayoutDirection::VERTICAL || children.size() == 1)
        {
            if (child->centerX)
            {
                int childWidth = (child->wType == RectType::FILL) ? availableWidth : child->width;
                child->xOffset = (availableWidth - childWidth) / 2;
                if (child->xOffset < 0) child->xOffset = 0;
            }
        }
        if (layoutDirection == LayoutDirection::HORIZONTAL || children.size() == 1)
        {
            if (child->centerY)
            {
                int childHeight = (child->hType == RectType::FILL) ? availableHeight : child->height;
                child->yOffset = (availableHeight - childHeight) / 2;
                if (child->yOffset < 0) child->yOffset = 0;
            }
        }

        if (layoutDirection == LayoutDirection::VERTICAL)
        {
            childRect.x = cursor.x + child->xOffset;
            childRect.y = cursor.y + child->yOffset;

            if (child->wType == RectType::FILL)
            {
                childRect.w = (bounds.x - childRect.x);
                if (childRect.w < 0) childRect.w = 0;
            }
            else
            {
                childRect.w = child->width;
            }

            if (child->hType == RectType::FILL)
            {
                childRect.h = fillSizePerElement + (remainingSize > 0 ? 1 : 0) - child->yOffset;
                if (childRect.h < 0) childRect.h = 0;
                remainingSize--;
            }
            else
            {
                childRect.h = child->height;
            }

            if (childRect.x + childRect.w > bounds.x) childRect.w = bounds.x - childRect.x;
            if (childRect.y + childRect.h > bounds.y) childRect.h = bounds.y - childRect.y;
            if (childRect.w < 0) childRect.w = 0;
            if (childRect.h < 0) childRect.h = 0;

            cursor.y += childRect.h + child->yOffset;
        }
        else
        {
            childRect.x = cursor.x + child->xOffset;
            childRect.y = cursor.y + child->yOffset;

            if (child->wType == RectType::FILL)
            {
                childRect.w = fillSizePerElement + (remainingSize > 0 ? 1 : 0) - child->xOffset;
                if (childRect.w < 0) childRect.w = 0;
                remainingSize--;
            }
            else
            {
                childRect.w = child->width;
            }

            if (child->hType == RectType::FILL)
            {
                childRect.h = (bounds.y - childRect.y);
                if (childRect.h < 0) childRect.h = 0;
            }
            else
            {
                childRect.h = child->height;
            }

            if (childRect.x + childRect.w > bounds.x) childRect.w = bounds.x - childRect.x;
            if (childRect.y + childRect.h > bounds.y) childRect.h = bounds.y - childRect.y;
            if (childRect.w < 0) childRect.w = 0;
            if (childRect.h < 0) childRect.h = 0;

            cursor.x += childRect.w + child->xOffset;
        }

        child->SetRect(childRect);
    }
}

int SableUI::Element::GetWidth(bool surface)
{
    if (wType == RectType::FIXED) return width + xOffset;
    if (children.empty())
    {
        return std::max(width, 0);
    }

    int calcWidth = xOffset;

    if (layoutDirection == LayoutDirection::VERTICAL)
    {
        int maxWidth = 0;
        for (Element* child : children)
        {
            maxWidth = std::max(maxWidth, child->GetWidth(false));
        }
        calcWidth = maxWidth;
    }
    else
    {
        int totalWidth = 0;
        for (Element* child : children)
        {
            totalWidth += child->GetWidth(false);
        }
        if (wType == RectType::FIXED) calcWidth = std::max(width, totalWidth);
        else calcWidth = totalWidth;
    }

    if (wType == RectType::FIT_CONTENT)
    {
        calcWidth += 2 * padding;
    }
    else if (surface)
    {
        calcWidth += 2 * padding;
    }

    return std::max(calcWidth, 0);
}

int SableUI::Element::GetHeight(bool surface)
{
    if (hType == RectType::FIXED) return height + yOffset;
    if (children.empty())
    {
        return std::max(height, 0);
    }

    int calcHeight = yOffset;

    if (layoutDirection == LayoutDirection::VERTICAL)
    {
        int totalHeight = 0;
        for (Element* child : children)
        {
            totalHeight += child->GetHeight(false);
        }
        if (hType == RectType::FIXED) calcHeight = std::max(height, totalHeight);
        else calcHeight = totalHeight;
    }
    else
    {
        int maxHeight = 0;
        for (Element* child : children)
        {
            maxHeight = std::max(maxHeight, child->GetHeight(false));
        }
        calcHeight = maxHeight;
    }

    if (hType == RectType::FIT_CONTENT)
    {
        calcHeight += 2 * padding;
    }
    else if (surface)
	{
		calcHeight += 2 * padding;
	}

    return std::max(calcHeight, 0);
}

SableUI::Element::~Element()
{
    delete drawable;
}