#include <SDL.h>
#include "SBUI_Component.h"
#include "SBUI_Renderer.h"

BaseComponent::BaseComponent(SableUI::colour colour, float border, SableUI::colour bColour)
{
	this->colour = colour;
	this->border = border;
	this->renderer = &SableUI::Renderer::Get();
	this->bColour = bColour;
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