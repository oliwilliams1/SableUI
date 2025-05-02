#include <SDL.h>
#include "SBUI_Component.h"
#include "SBUI_Renderer.h"

void BaseComponent::Render()
{
	SbUI_Rect r = { parent->xPx, parent->yPx, parent->wPx, parent->hPx };
	SbUI_Renderer::Get().DrawRect(r, colour);
}