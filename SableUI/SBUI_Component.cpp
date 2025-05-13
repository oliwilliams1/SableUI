#include <SDL.h>
#include "SBUI_Component.h"
#include "SBUI_Renderer.h"

BaseComponent::BaseComponent(SableUI::colour colour)
{
	this->colour = colour;
}

void BaseComponent::UpdateDrawable()
{
	if (renderer == nullptr)
	{
		renderer = &SableUI::Renderer::Get();
	}

	float bSize = 0.0f;

	if (parent->parent != nullptr)
	{
		bSize = parent->parent->bSize; // Components parent
	}

	drawable.Update(parent->rect, colour, bSize, true);
}

void BaseComponent::Render()
{
    if (renderer == nullptr)
    {
        renderer = &SableUI::Renderer::Get();
    }

    renderer->Draw(std::make_unique<Drawable::Base>(drawable));
}