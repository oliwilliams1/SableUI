#include "SableUI/component.h"

#include <SDL.h>
#include "SableUI/renderer.h"

static SableUI::Renderer* renderer = nullptr;

/* - Default solid component - */

void DefaultComponent::UpdateDrawable()
{
	float bSize = 0.0f;

	if (parent->parent != nullptr)
	{
		bSize = parent->parent->bSize;
	}

	drawable.Update(parent->rect, colour, bSize, false);

	Render();

	UpdateElements();
}

void DefaultComponent::Render()
{
    if (renderer == nullptr) renderer = &SableUI::Renderer::Get();

    renderer->Draw(std::make_unique<Drawable::Rect>(drawable));

	RenderElements();
}

void DefaultComponent::RenderElements()
{
	for (const auto& e : elements)
	{
		e.get()->Render();
	}
}

void DefaultComponent::AddElement(std::unique_ptr<BaseElement>& e)
{
	elements.push_back(std::move(e));
}

void DefaultComponent::UpdateElements()
{
	SableUI::vec2 cursor = { SableUI::f2i(parent->rect.x),
							SableUI::f2i(parent->rect.y) };

	SableUI::vec2 bounds = { SableUI::f2i(parent->rect.x + parent->rect.w),
							SableUI::f2i(parent->rect.y + parent->rect.h) };

	for (const auto& e : elements)
	{

		SableUI::rect elementRect = e.get()->r;

		elementRect.y = cursor.y;
		elementRect.x = parent->rect.x;

		cursor.y += elementRect.h;

		elementRect.h = 12;

		e.get()->SetRect(elementRect);
	}
};

/* - Splitter - */

void SplitterComponent::UpdateDrawable()
{
	std::vector<int> segments;

	for (SableUI_node* child : parent->children)
	{
		if (parent->type == NodeType::HSPLITTER)
		{
			segments.push_back(SableUI::f2i(child->rect.x - parent->rect.x));
		}

		if (parent->type == NodeType::VSPLITTER)
		{
			segments.push_back(SableUI::f2i(child->rect.y - parent->rect.y));
		}
	}

	drawable.Update(parent->rect, bColour, parent->type, parent->bSize, segments, true);
}

void SplitterComponent::Render()
{
	if (renderer == nullptr) renderer = &SableUI::Renderer::Get();

	renderer->Draw(std::make_unique<Drawable::bSplitter>(drawable));
}