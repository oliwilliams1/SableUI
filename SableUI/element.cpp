#include "SableUI/element.h"

static SableUI::Renderer* renderer = nullptr;

using namespace SableUI;

/* Base Element */
void SableUI::BaseElement::SetRect(const rect& rect)
{
	this->drawableRect = rect;
	this->bgDrawable.Update(drawableRect, bgColour, 0.0f, true);
}

void SableUI::BaseElement::SetInfo(const ElementInfo& info)
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
}

void SableUI::BaseElement::Render(int z)
{
	if (renderer == nullptr) renderer = &Renderer::Get();

	bgDrawable.setZ(z);
	renderer->Draw(std::make_unique<SableUI_Drawable::Rect>(bgDrawable));

	AdditionalRender();

	UpdateChildren();
    for (BaseElement* element : children)
	{
        element->Render(z + 1);
    }
	if (children.size() == 0) return;

}

void SableUI::BaseElement::UpdateChildren()
{
	/* use cursor to place elements */
	vec2 cursor = { SableUI::f2i(drawableRect.x),
							 SableUI::f2i(drawableRect.y) };

	vec2 bounds = { SableUI::f2i(drawableRect.x + drawableRect.w),
							 SableUI::f2i(drawableRect.y + drawableRect.h) };

	for (BaseElement* child : children)
	{
		if (child->wType == RectType::FIXED && child->centerX)
		{
			child->xOffset = (drawableRect.w - child->width) / 2.0f;
		}

		if (child->hType == RectType::FIXED && child->centerY)
		{
			child->yOffset = (drawableRect.h - child->height) / 2.0f;
		}

		SableUI::rect tempElRect = { 0, 0, 0, 0 };
		tempElRect.y = cursor.y + child->yOffset;
		tempElRect.x = cursor.x + child->xOffset;
		tempElRect.w = child->width;
		tempElRect.h = child->height;

		/* calc fill type */
		if (child->wType == RectType::FILL) tempElRect.w = drawableRect.w;

		if (child->hType == RectType::FILL)
		{
			float fillHeight = drawableRect.h;
			float fillCtr = 0;

			for (BaseElement* child2 : children)
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

void SableUI::BaseElement::AddChild(BaseElement* child)
{
	children.push_back(child);
}

/* Element arena */
std::vector<BaseElement*> ElementArena::elements;

BaseElement* ElementArena::CreateElement(const std::string& name)
{
    for (BaseElement* element : elements)
    {
        if (element->name == name)
        {
            SableUI_Error("Element already exists: %s", name.c_str());
            return nullptr;
        }
    }

    BaseElement* element = new BaseElement(name);
    elements.push_back(element);

    return element;
}

BaseElement* ElementArena::GetElement(const std::string& name)
{
    for (BaseElement* element : elements)
    {
        if (element->name == name)
        {
            return element;
        }
    }

    return nullptr;
}

void ElementArena::ShutdownArena()
{
    for (BaseElement* element : elements)
    {
        delete element;
    }

    elements.clear();
}
