#include <SDL.h>
#include "SBUI_Component.h"
#include "SBUI_Renderer.h"

BaseComponent::BaseComponent(SableUI::colour colour, float border, SableUI::colour bColour, 
	Drawable::size s)
{
	this->colour = colour;
	this->border = border;
	this->renderer = &SableUI::Renderer::Get();
	this->bColour = bColour;
	this->size = s;

	if (s.v != -1.0f && size.type == Drawable::SizeType::PERCENT)
	{
		if (parent == nullptr) return;

		if (parent->type == NodeType::VSPLITTER)
		{
			parent->scaleFac.y = s.v / 100.0f;
		}

		if (parent->type == NodeType::HSPLITTER)
		{
			parent->scaleFac.x = s.v / 100.0f;
		}
	}
}

void BaseComponent::UpdateDrawable()
{
	if (renderer == nullptr)
	{
		renderer = &SableUI::Renderer::Get();
	}

	drawable.Update(parent->rect, colour, border, bColour);
}

void BaseComponent::Render()
{
    if (renderer == nullptr)
    {
        renderer = &SableUI::Renderer::Get();
    }

    renderer->Draw(std::make_unique<Drawable::Base>(drawable));
}