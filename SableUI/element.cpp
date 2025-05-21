#include "SableUI/element.h"

static SableUI::Renderer* renderer = nullptr;

void SableUI::BaseElement::SetRect(const SableUI::rect& rect)
{
	this->drawableRect = rect;
	this->bgDrawable.Update(drawableRect, bgColour, 0.0f, true);
}

void SableUI::BaseElement::Render()
{
	if (renderer == nullptr) renderer = &SableUI::Renderer::Get();

	renderer->Draw(std::make_unique<SableUI_Drawable::Rect>(bgDrawable));

	AdditionalRender();
}