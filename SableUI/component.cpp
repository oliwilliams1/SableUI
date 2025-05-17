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

	drawable.Update(parent->rect, colour, bSize, true);
}

void DefaultComponent::Render()
{
    if (renderer == nullptr) renderer = &SableUI::Renderer::Get();

    renderer->Draw(std::make_unique<Drawable::Rect>(drawable));
}

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

void BaseComponent::AddElement(std::unique_ptr<BaseElement>& e)
{
	elements.push_back(std::move(e));
}
