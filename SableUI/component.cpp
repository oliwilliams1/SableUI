#include "SableUI/component.h"
#include "SableUI/SableUI.h"

SableUI::BaseComponent::BaseComponent(Colour colour)
{
	m_bgColour = colour;
}

void SableUI::BaseComponent::BackendInitialise(Renderer* renderer)
{
	if (rootElement) delete rootElement;

	m_renderer = renderer;
	
	rootElement = new Element(renderer, ElementType::DIV);
	rootElement->Init(renderer, ElementType::DIV);
	rootElement->setBgColour(m_bgColour);
	
	SetElementBuilderContext(renderer, rootElement);
	Layout();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	return rootElement;
}

void SableUI::BaseComponent::Rerender()
{
	BackendInitialise(m_renderer);
	needsRerender = false;
}

bool SableUI::BaseComponent::comp_PropagateComponentStateChanges()
{
	bool neededRerender = needsRerender;
	if (needsRerender)
	{
		Rerender();
		rootElement->LayoutChildren();
	}

	bool res = rootElement->el_PropagateComponentStateChanges();

	return res || neededRerender;
}