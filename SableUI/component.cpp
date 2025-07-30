#include "SableUI/component.h"

SableUI::BaseComponent::BaseComponent(Renderer* renderer)
{
	m_baseElement.bgColour = { 80, 0, 0 };
	m_baseElement.Init(renderer, ElementType::RECT);
}

SableUI::Element* SableUI::BaseComponent::GetBaseElement()
{
	return &m_baseElement;
}