#include "SableUI/component.h"
#include "SableUI/SableUI.h"

SableUI::BaseComponent::BaseComponent(Colour colour)
{
	rootElement.bgColour = colour;
}

void SableUI::BaseComponent::BackendInitialise(Renderer* renderer)
{
	rootElement.Init(renderer, ElementType::DIV);
	SetElementBuilderContext(renderer, &rootElement);
	Init();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	return &rootElement;
}