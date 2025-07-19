#include "SableUI/element.h"

using namespace SableUI;

SableUI::Element::Element(const std::string name, Renderer* renderer, ElementType type)
    : name(name), renderer(renderer), type(type)
{
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
    this->width	= info.width;
    this->height = info.height;
    this->padding = info.padding;
    this->wType = info.wType;
    this->hType = info.hType;
    this->centerX = info.centerX;
    this->centerY = info.centerY;
    this->bgColour	= info.bgColour;
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
void SableUI::Element::UpdateChildren()
{
    if (type == ElementType::IMAGE || type == ElementType::TEXT) return;

    vec2 cursor = { SableUI::f2i(rect.x), SableUI::f2i(rect.y) };
    vec2 bounds = { SableUI::f2i(rect.x + rect.w), SableUI::f2i(rect.y + rect.h) };

    if (wType == RectType::FIT_CONTENT)
    {
        width = GetWidth();
    }
    if (hType == RectType::FIT_CONTENT)
    {
        height = GetHeight();
    }

    float totalSize = 0.0f;
    float fillCount = 0.0f;

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

    float availableSize = (layoutDirection == LayoutDirection::VERTICAL) ? rect.h : rect.w;
    float fillSizePerElement = 0.0f;

    if (fillCount > 0)
    {
        fillSizePerElement = (availableSize - totalSize) / fillCount;
        if (fillSizePerElement < 0) fillSizePerElement = 0;
    }

    for (Element* child : children)
    {
        SableUI::Rect childRect = { 0, 0, 0, 0 };

        if (layoutDirection == LayoutDirection::VERTICAL || children.size() == 1)
        {
            if (child->centerX)
            {
                float availableWidth = rect.w;
                float childWidth = (child->wType == RectType::FILL) ? availableWidth : child->width;
                child->xOffset = (availableWidth - childWidth) / 2.0f;
                if (child->xOffset < 0) child->xOffset = 0;
            }
        }
        if (layoutDirection == LayoutDirection::HORIZONTAL || children.size() == 1)
        {
            if (child->centerY)
            {
                float availableHeight = rect.h;
                float childHeight = (child->hType == RectType::FILL) ? availableHeight : child->height;
                child->yOffset = (availableHeight - childHeight) / 2.0f;
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
                childRect.h = fillSizePerElement - child->yOffset;
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

            cursor.y += childRect.h + child->yOffset;
        }
        else
        {
            childRect.x = cursor.x + child->xOffset;
            childRect.y = cursor.y + child->yOffset;

            if (child->wType == RectType::FILL)
            {
                childRect.w = fillSizePerElement - child->xOffset;
                if (childRect.w < 0) childRect.w = 0;
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

float SableUI::Element::GetWidth()
{
    if (children.empty())
    {
        if (wType == RectType::FIXED || wType == RectType::FIT_CONTENT)
        {
            return width;
        }
        else
        {
            return (width > 0) ? width : 0.0f;
        }
    }

    float calcWidth = 0.0f;

    if (layoutDirection == LayoutDirection::VERTICAL)
    {
        float maxWidth = 0.0f;
        for (Element* child : children)
        {
            maxWidth = std::max(maxWidth, child->GetWidth());
        }
        calcWidth = maxWidth;
    }
    else
    {
        float totalWidth = 0.0f;
        for (Element* child : children)
        {
            totalWidth += child->GetWidth();
        }
        calcWidth = totalWidth;
    }

    if (wType == RectType::FIXED || wType == RectType::FIT_CONTENT)
    {
        return std::max(width, calcWidth);
    }
    else
    {
        return std::max(calcWidth, 0.0f);
    }
}

float SableUI::Element::GetHeight()
{
    if (children.empty())
    {
        if (hType == RectType::FIXED || hType == RectType::FIT_CONTENT)
        {
            return height;
        }
        else
        {
            return (height > 0) ? height : 0.0f;
        }
    }


    float calcHeight = 0.0f;

    if (layoutDirection == LayoutDirection::VERTICAL)
    {
        float totalHeight = 0.0f;
        for (Element* child : children)
        {
            totalHeight += child->GetHeight();
        }
        calcHeight = totalHeight;
    }
    else
    {
        float maxHeight = 0.0f;
        for (Element* child : children)
        {
            maxHeight = std::max(maxHeight, child->GetHeight());
        }
        calcHeight = maxHeight;
    }

    if (hType == RectType::FIXED || hType == RectType::FIT_CONTENT)
    {
        return std::max(height, calcHeight);
    }
    else
    {
        return std::max(calcHeight, 0.0f);
    }
}

SableUI::Element::~Element()
{
    delete drawable;
}