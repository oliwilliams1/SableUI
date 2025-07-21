#include "SableUI/component.h"
#include "SableUI/renderer.h"

static SableUI::Renderer* renderer = nullptr;

/* - Default solid component - */

void SableUI::Body::UpdateDrawable(bool draw)
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

void SableUI::Body::Render()
{
    renderer->Draw(&drawable);

	RenderElements();
}

void SableUI::Body::RenderElements()
{
    m_element->Render();
}

void SableUI::Body::AddElement(Element* e)
{
    if (m_element == nullptr)
    {
        m_element = e;
        return;
    }
    SableUI_Error("Cannot add more than one element to body");
}

void SableUI::Body::UpdateElements()
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

    m_element->SetRect(Rect(cursor.x, cursor.y, bounds.x - cursor.x, bounds.y - cursor.y));
}

/* - Splitter - */
void SableUI::Splitter::UpdateDrawable(bool draw)
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

void SableUI::Splitter::Render()
{
	renderer->Draw(&drawable);
}