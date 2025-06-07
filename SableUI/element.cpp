#include "SableUI/element.h"

using namespace SableUI;

SableUI::Element::Element(const std::string name, Renderer* renderer, ElementType type) : name(name), renderer(renderer)
{
	drawable = new DrawableRect();
}

void SableUI::Element::SetRect(const Rect& r)
{
	this->rect = r;

	switch (type)
	{
	case ElementType::RECT:
		if (DrawableRect* drRect = dynamic_cast<DrawableRect*>(drawable))
		{
			renderer->Draw(drRect);
			drRect->Update(rect, bgColour, 0.0f);
		}
		else
		{
			SableUI_Error("Dynamic cast failed");
		}
		break;

	case ElementType::IMAGE:
		if (DrawableImage* drImage = dynamic_cast<DrawableImage*>(drawable))
		{
			renderer->Draw(drImage);
			drImage->Update(rect);
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
    this->name     = info.name;
    this->xOffset  = info.xOffset;
    this->yOffset  = info.yOffset;
    this->width    = info.width;
    this->height   = info.height;
    this->padding  = info.padding;
    this->wType    = info.wType;
    this->hType    = info.hType;
    this->centerX  = info.centerX;
    this->centerY  = info.centerY;
    this->bgColour = info.bgColour;
	this->type     = info.type;
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

			AdditionalRender();

			UpdateChildren();
			for (Element* element : children)
			{
				element->Render(z + 1);
			}
			if (children.size() == 0) return;
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
			SableUI_Warn("Image element not implemented yet!");
		}
		else
		{
			SableUI_Error("Dynamic cast failed");
		}
		break;
	}
	}
}

void SableUI::Element::UpdateChildren()
{
	if (type == ElementType::IMAGE) return;

	/* use cursor to place elements */
	vec2 cursor = { SableUI::f2i(rect.x),
							 SableUI::f2i(rect.y) };

	vec2 bounds = { SableUI::f2i(rect.x + rect.w),
							 SableUI::f2i(rect.y + rect.h) };

	for (Element* child : children)
	{
		if (child->wType == RectType::FIXED && child->centerX)
		{
			child->xOffset = (rect.w - child->width) / 2.0f;
		}

		if (child->hType == RectType::FIXED && child->centerY)
		{
			child->yOffset = (rect.h - child->height) / 2.0f;
		}

		SableUI::Rect tempElRect = { 0, 0, 0, 0 };
		tempElRect.y = cursor.y + child->yOffset;
		tempElRect.x = cursor.x + child->xOffset;
		tempElRect.w = child->width;
		tempElRect.h = child->height;

		/* calc fill type */
		if (child->wType == RectType::FILL) tempElRect.w = rect.w;

		if (child->hType == RectType::FILL)
		{
			float fillHeight = rect.h;
			float fillCtr = 0;

			for (Element* child2 : children)
			{
				if (child2->hType == RectType::FIXED) fillHeight -= child2->height;
				else fillCtr++;
			}
			tempElRect.h = fillHeight / fillCtr;
		}

		/* max to bounds */
		if (tempElRect.x + tempElRect.w > bounds.x) tempElRect.w = bounds.x - tempElRect.x;
		if (tempElRect.y + tempElRect.h > bounds.y) tempElRect.h = bounds.y - tempElRect.y;

		/* upd cursor */
		cursor.y += tempElRect.h + child->yOffset;

		/* calc padding */
		if (child->padding > 0.0f)
		{
			tempElRect.x += child->padding;
			tempElRect.y += child->padding;
			tempElRect.w -= 2.0f * child->padding;
			tempElRect.h -= 2.0f * child->padding;
		}

		child->SetRect(tempElRect);
	}
}

void SableUI::Element::AddChild(Element* child)
{
	if (type == ElementType::IMAGE) SableUI_Error("Cannot add children to element of type image");
	children.push_back(child);
}

SableUI::Element::~Element()
{
	delete drawable;
}