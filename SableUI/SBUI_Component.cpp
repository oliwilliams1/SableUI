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

	renderer->GetDrawableRect(drawableRect, parent->rect, colour, border);
	renderer->rectQueue.push_back(drawableRect);

	if (border > 0.0f)
	{
		renderer->GetDrawableRectBorder(drawableRectBorder, parent->rect, SableUI::colour(0, 0, 0, 0), border);
		renderer->rectBorderQueue.push_back(drawableRectBorder);
	}
}

void BaseComponent::Render()
{
	if (renderer == nullptr)
	{
		renderer = &SableUI::Renderer::Get();
	}

	renderer->rectQueue.push_back(drawableRect);

	if (drawableRectBorder.border > 0.0f)
	{
		renderer->rectBorderQueue.push_back(drawableRectBorder);
	}
}