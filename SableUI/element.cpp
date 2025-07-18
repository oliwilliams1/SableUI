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

	/* use cursor to place elements */
	vec2 cursor = { SableUI::f2i(rect.x),
							 SableUI::f2i(rect.y) };

	vec2 bounds = { SableUI::f2i(rect.x + rect.w),
							 SableUI::f2i(rect.y + rect.h) };

	float totalFixedMainAxisDimension = 0.0f;
	float fillMainAxisCount = 0.0f;

	for (Element* child : children)
	{
		if (layoutDirection == LayoutDirection::VERTICAL)
		{
			if (child->hType == RectType::FIXED)
			{
				totalFixedMainAxisDimension += child->height + (2.0f * child->padding);
			}
			else if (child->hType == RectType::FILL)
			{
				fillMainAxisCount++;
			}
		}
		else
		{
			if (child->wType == RectType::FIXED)
			{
				totalFixedMainAxisDimension += child->width + (2.0f * child->padding);
			}
			else if (child->wType == RectType::FILL)
			{
				fillMainAxisCount++;
			}
		}
	}

	float availableMainAxisDimension = (layoutDirection == LayoutDirection::VERTICAL) ? rect.h : rect.w;
	float fillMainAxisUnitDimension = 0.0f;
	if (fillMainAxisCount > 0)
	{
		fillMainAxisUnitDimension = (availableMainAxisDimension - totalFixedMainAxisDimension) / fillMainAxisCount;
		if (fillMainAxisUnitDimension < 0) fillMainAxisUnitDimension = 0;
	}

	for (Element* child : children)
	{
		SableUI::Rect tempElRect = { 0, 0, 0, 0 };

		if (layoutDirection == LayoutDirection::VERTICAL)
		{
			if (child->wType == RectType::FIXED && child->centerX)
			{
				child->xOffset = (rect.w - child->width) / 2.0f;
			}
		}
		else
		{
			if (child->hType == RectType::FIXED && child->centerY)
			{
				child->yOffset = (rect.h - child->height) / 2.0f;
			}
		}


		if (layoutDirection == LayoutDirection::VERTICAL)
		{
			tempElRect.y = cursor.y + child->yOffset;
			tempElRect.x = cursor.x + child->xOffset;

			if (child->wType == RectType::FILL)
			{
				tempElRect.w = rect.w;
			}
			else
			{
				tempElRect.w = child->width;
			}

			if (child->hType == RectType::FILL)
			{
				tempElRect.h = fillMainAxisUnitDimension - (2.0f * child->padding);
				if (tempElRect.h < 0) tempElRect.h = 0;
			}
			else
			{
				tempElRect.h = child->height;
			}

			if (tempElRect.x + tempElRect.w > bounds.x) tempElRect.w = bounds.x - tempElRect.x;
			if (tempElRect.y + tempElRect.h > bounds.y) tempElRect.h = bounds.y - tempElRect.y;

			cursor.y += tempElRect.h + child->yOffset + (2.0f * child->padding);
		}
		else
		{
			tempElRect.x = cursor.x + child->xOffset;
			tempElRect.y = cursor.y + child->yOffset;

			if (child->wType == RectType::FILL)
			{
				tempElRect.w = fillMainAxisUnitDimension - (2.0f * child->padding);
				if (tempElRect.w < 0) tempElRect.w = 0;
			}
			else
			{
				tempElRect.w = child->width;
			}

			if (child->hType == RectType::FILL)
			{
				tempElRect.h = rect.h;
			}
			else
			{
				tempElRect.h = child->height;
			}

			// clamp to bounds
			if (tempElRect.x + tempElRect.w > bounds.x) tempElRect.w = bounds.x - tempElRect.x;
			if (tempElRect.y + tempElRect.h > bounds.y) tempElRect.h = bounds.y - tempElRect.y;

			cursor.x += tempElRect.w + child->xOffset + (2.0f * child->padding);
		}

		/* calc padding */
		if (child->padding > 0.0f)
		{
			tempElRect.x += child->padding;
			tempElRect.y += child->padding;
			tempElRect.w -= 2.0f * child->padding;
			tempElRect.h -= 2.0f * child->padding;
			
			if (tempElRect.w < 0) tempElRect.w = 0;
			if (tempElRect.h < 0) tempElRect.h = 0;
		}

		child->SetRect(tempElRect);
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
		drImage->m_texture.LoadTextureOptimised(path, width - 2.0f * padding, height - 2.0f * padding);
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
		if (wType == RectType::FIXED)
		{
			return width;
		}
		else
		{
			return (width > 0) ? width : 20.0f;
		}
	}

	float calculatedWidth = 0.0f;

	if (layoutDirection == LayoutDirection::VERTICAL)
	{
		float maxChildrenContentWidth = 0.0f;
		for (Element* child : children)
		{
			maxChildrenContentWidth = std::max(maxChildrenContentWidth, child->GetWidth() + (2.0f * child->padding));
		}
		calculatedWidth = maxChildrenContentWidth;
	}
	else
	{
		float totalChildrenContentWidth = 0.0f;
		for (Element* child : children)
		{
			totalChildrenContentWidth += child->GetWidth() + (2.0f * child->padding);
		}
		calculatedWidth = totalChildrenContentWidth;
	}

	if (wType == RectType::FIXED)
	{
		return std::max(width, calculatedWidth);
	}
	else
	{
		return std::max(calculatedWidth, 20.0f);
	}
}


float SableUI::Element::GetHeight()
{
	if (children.empty())
	{
		if (hType == RectType::FIXED)
		{
			return height;
		}
		else
		{
			return (height > 0) ? height : 20.0f;
		}
	}

	float calculatedHeight = 0.0f;

	if (layoutDirection == LayoutDirection::VERTICAL)
	{
		float totalChildrenContentHeight = 0.0f;
		for (Element* child : children)
		{
			totalChildrenContentHeight += child->GetHeight() + (2.0f * child->padding);
		}
		calculatedHeight = totalChildrenContentHeight;
	}
	else
	{
		float maxChildrenContentHeight = 0.0f;
		for (Element* child : children)
		{
			maxChildrenContentHeight = std::max(maxChildrenContentHeight, child->GetHeight() + (2.0f * child->padding));
		}
		calculatedHeight = maxChildrenContentHeight;
	}

	if (hType == RectType::FIXED)
	{
		return std::max(height, calculatedHeight);
	}
	else
	{
		return std::max(calculatedHeight, 20.0f);
	}
}

SableUI::Element::~Element()
{
	delete drawable;
}