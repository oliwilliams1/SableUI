#include <SDL.h>
#include "SBUI_Component.h"
#include "SBUI_Renderer.h"

void BaseComponent::Render()
{
	SableUI::Renderer::Get().DrawRect(parent->rect, colour);
}