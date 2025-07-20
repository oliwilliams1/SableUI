#include "SableUI/component.h"
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

	drawable.Update(parentNode->rect, colour, bSize);

	UpdateElements();

	if (draw) Render();
}

void SableUI::DefaultComponent::Render()
{
    renderer->Draw(&drawable);

	RenderElements();
}

void SableUI::DefaultComponent::RenderElements()
{
	for (Element* element : elements)
	{
		element->Render();
	}
}

void SableUI::DefaultComponent::AddElement(Element* e)
{
	elements.push_back(e);
}

void SableUI::DefaultComponent::UpdateElements()
{
    if (parentNode == nullptr)
    {
        SableUI_Warn("Cannot update elements for null parent node");
        return;
    }

    /* use cursor to place elements */
    vec2 cursor = { SableUI::f2i(parentNode->rect.x),
                    SableUI::f2i(parentNode->rect.y) };
    vec2 bounds = { SableUI::f2i(parentNode->rect.x + parentNode->rect.w),
                    SableUI::f2i(parentNode->rect.y + parentNode->rect.h) };

    for (Element* element : elements)
    {
        if (parentNode != nullptr && (element->wType == RectType::FIXED || element->wType == RectType::FIT_CONTENT) && element->centerX)
        {
            element->xOffset = (parentNode->rect.w - element->width) / 2.0f;
            if (element->xOffset < 0) element->xOffset = 0;
        }
        if (parentNode != nullptr && (element->hType == RectType::FIXED || element->hType == RectType::FIT_CONTENT) && element->centerY)
        {
            element->yOffset = (parentNode->rect.h - element->height) / 2.0f;
            if (element->yOffset < 0) element->yOffset = 0;
        }

        SableUI::Rect tempElRect = { 0, 0, 0, 0 };
        tempElRect.y = cursor.y + element->yOffset;
        tempElRect.x = cursor.x + element->xOffset;
        tempElRect.w = element->width;
        tempElRect.h = element->height;

        /* apply border */
        int bSize = parentNode->bSize;
        if (parentNode != nullptr && parentNode->parent != nullptr)
        {
            bSize = parentNode->parent->bSize;
        }
        tempElRect.x += bSize;
        tempElRect.y += bSize;

        /* calc fill type */
        if (parentNode != nullptr)
        {
            if (element->wType == RectType::FILL)
            {
                tempElRect.w = parentNode->rect.w - 2 * bSize;
                if (tempElRect.w < 0) tempElRect.w = 0;
            }
            if (element->hType == RectType::FILL)
            {
                int fillHeight = parentNode->rect.h - 2 * bSize;
                int fillCtr = 0;

                for (Element* el2 : elements)
                {
                    if (el2->hType == RectType::FIXED || el2->hType == RectType::FIT_CONTENT)
                        fillHeight -= el2->height;
                    else
                        fillCtr++;
                }

                if (fillCtr > 0)
                {
                    tempElRect.h = fillHeight / fillCtr;
                    if (tempElRect.h < 0) tempElRect.h = 0;
                }
                else
                {
                    tempElRect.h = 0;
                }
            }
        }

        /* max to bounds */
        if (tempElRect.x + tempElRect.w > bounds.x)
        {
            tempElRect.w = bounds.x - tempElRect.x;
            if (tempElRect.w < 0) tempElRect.w = 0;
        }
        if (tempElRect.y + tempElRect.h > bounds.y)
        {
            tempElRect.h = bounds.y - tempElRect.y;
            if (tempElRect.h < 0) tempElRect.h = 0;
        }

        /* upd cursor */
        cursor.y += tempElRect.h + element->yOffset;
        element->SetRect(tempElRect);
    }
}

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

	drawable.Update(parentNode->rect, bColour, parentNode->type, parentNode->bSize, segments);

	if (draw) Render();
}

void SableUI::SplitterComponent::Render()
{
	renderer->Draw(&drawable);
}