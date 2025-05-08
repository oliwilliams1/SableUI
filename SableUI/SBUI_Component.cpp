#include <SDL.h>
#include "SBUI_Component.h"
#include "SBUI_Renderer.h"

BaseComponent::BaseComponent(SableUI::colour colour, float border)
{
	this->colour = colour;
	this->border = border;
	this->renderer = &SableUI::Renderer::Get();
}

void BaseComponent::UpdateDrawable()
{
	if (renderer == nullptr)
	{
		renderer = &SableUI::Renderer::Get();
	}
	drawableRect = renderer->GetDrawableRect(parent->rect, colour, border);
	renderer->rectQueue.push_back(drawableRect);
}

void BaseComponent::Render()
{
	if (renderer == nullptr)
	{
		renderer = &SableUI::Renderer::Get();
	}

	renderer->rectQueue.push_back(drawableRect);
}