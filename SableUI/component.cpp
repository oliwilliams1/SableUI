#include "SableUI/component.h"
#include "SableUI/SableUI.h"

SableUI::BaseComponent::BaseComponent(Colour colour)
{
	rootElement.bgColour = colour;
}

void SableUI::BaseComponent::BackendInitialise(Renderer* renderer)
{
	m_renderer = renderer;
	rootElement.Init(renderer, ElementType::DIV);
	SetElementBuilderContext(renderer, &rootElement);
	Layout();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	return &rootElement;
}

void SableUI::BaseComponent::Rerender()
{
	rootElement.children.clear();

	SetElementBuilderContext(m_renderer, &rootElement);
	Layout();
}
