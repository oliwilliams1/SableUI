#include "SableUI/component.h"

#include <SDL.h>
#include "SableUI/renderer.h"

static SableUI::Renderer* renderer = nullptr;

/* - Default solid component - */

void SableUI::DefaultComponent::UpdateDrawable(bool draw)
{
	float bSize = 0.0f;

	if (parentNode->parent != nullptr)
	{
		bSize = parentNode->parent->bSize;
	}

	drawable.Update(parentNode->rect, colour, bSize, false);

	if (draw) Render();

	UpdateElements();
}

void SableUI::DefaultComponent::Render()
{
    if (renderer == nullptr) renderer = &Renderer::Get();

    renderer->Draw(std::make_unique<SableUI_Drawable::Rect>(drawable));

	RenderElements();
}

void SableUI::DefaultComponent::RenderElements()
{
	for (BaseElement* element : elements)
	{
		element->Render();
	}
}

void SableUI::DefaultComponent::AddElement(BaseElement* e)
{
	elements.push_back(e);
}

void SableUI::DefaultComponent::UpdateElements()
{
	/* use cursor to place elements */
	vec2 cursor = { SableUI::f2i(parentNode->rect.x),
							 SableUI::f2i(parentNode->rect.y) };

	vec2 bounds = { SableUI::f2i(parentNode->rect.x + parentNode->rect.w),
							 SableUI::f2i(parentNode->rect.y + parentNode->rect.h) };

	for (BaseElement* element : elements)
	{
		if (parentNode != nullptr && element->wType == RectType::FIXED && element->centerX)
		{
			element->xOffset = (parentNode->rect.w - element->width) / 2.0f;
		}

		if (parentNode != nullptr && element->hType == RectType::FIXED && element->centerY)
		{
			element->yOffset = (parentNode->rect.h - element->height) / 2.0f;
		}

		SableUI::rect tempElRect = { 0, 0, 0, 0 };
		tempElRect.y = cursor.y + element->yOffset;
		tempElRect.x = cursor.x + element->xOffset;
		tempElRect.w = element->width;
		tempElRect.h = element->height;

		/* apply border */
		float bSize = 0.0f;
		if (parentNode != nullptr && parentNode->parent != nullptr)
		{
			bSize = parentNode->parent->bSize;
		}

		tempElRect.x += bSize;
		tempElRect.y += bSize;

		/* calc fill type */
		if (parentNode != nullptr)
		{
			if (element->wType == RectType::FILL) tempElRect.w = parentNode->rect.w - 2.0f * bSize;

			if (element->hType == RectType::FILL)
			{
				float fillHeight = parentNode->rect.h - 2.0f * bSize;
				float fillCtr = 0;

				for (BaseElement* el2 : elements)
				{
					if (el2->hType == RectType::FIXED) fillHeight -= el2->height;
					else fillCtr++;
				}
				tempElRect.h = fillHeight / fillCtr;
			}
		}

		/* max to bounds */
		if (tempElRect.x + tempElRect.w > bounds.x) tempElRect.w = bounds.x - tempElRect.x;
		if (tempElRect.y + tempElRect.h > bounds.y) tempElRect.h = bounds.y - tempElRect.y;

		/* upd cursor */
		cursor.y += tempElRect.h + element->yOffset;

		/* calc padding */
		if (element->padding > 0.0f)
		{
			tempElRect.x += element->padding;
			tempElRect.y += element->padding;
			tempElRect.w -= 2.0f * element->padding;
			tempElRect.h -= 2.0f * element->padding;
		}
		
		element->SetRect(tempElRect);
	}
};

/* - Splitter - */

void SableUI::SplitterComponent::UpdateDrawable(bool draw)
{
	std::vector<int> segments;

	/* calc segment nodes */
	for (SableUI::Node* child : parentNode->children)
	{
		if (parentNode->type == NodeType::HSPLITTER)
		{
			segments.push_back(SableUI::f2i(child->rect.x - parentNode->rect.x));
		}

		if (parentNode->type == NodeType::VSPLITTER)
		{
			segments.push_back(SableUI::f2i(child->rect.y - parentNode->rect.y));
		}
	}

	drawable.Update(parentNode->rect, bColour, parentNode->type, parentNode->bSize, segments, draw);
}

void SableUI::SplitterComponent::Render()
{
	if (renderer == nullptr) renderer = &Renderer::Get();

	renderer->Draw(std::make_unique<SableUI_Drawable::bSplitter>(drawable));
}