#include "SableUI/component.h"

SableUI::BaseComponent::BaseComponent(Colour colour)
{
	m_baseElement.bgColour = colour;
}

void SableUI::BaseComponent::SetRenderer(Renderer* renderer)
{
	m_baseElement.Init(renderer, ElementType::RECT);
}

SableUI::Element* SableUI::BaseComponent::GetBaseElement()
{
	return &m_baseElement;
}