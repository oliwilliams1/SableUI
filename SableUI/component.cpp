#include "SableUI/component.h"
#include "SableUI/SableUI.h"

SableUI::BaseComponent::BaseComponent(Colour colour)
{
	m_bgColour = colour;
}

SableUI::BaseComponent::~BaseComponent()
{
	for (BaseComponent* comp : m_componentChildren) delete comp;
	m_componentChildren.clear();
}

void SableUI::BaseComponent::BackendInitialisePanel(Renderer* renderer)
{
	if (rootElement) delete rootElement;

	m_renderer = renderer;
	
	rootElement = new Element(renderer, ElementType::DIV);
	rootElement->Init(renderer, ElementType::DIV);
	rootElement->setBgColour(m_bgColour);
	
	SetElementBuilderContext(renderer, rootElement);
	Layout();
}

void SableUI::BaseComponent::BackendInitialiseChild(BaseComponent* parent, const ElementInfo& info)
{
	if (rootElement) delete rootElement;

	m_renderer = parent->m_renderer;

	StartDiv(info);
	Layout();
	EndDiv();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	if (rootElement == nullptr) return nullptr;
	return rootElement;
}

void SableUI::BaseComponent::Rerender()
{
	if (rootElement) delete rootElement;

	rootElement = new Element(m_renderer, ElementType::DIV);
	rootElement->Init(m_renderer, ElementType::DIV);
	rootElement->setBgColour(m_bgColour);
	
	SetElementBuilderContext(m_renderer, rootElement);
	Layout();

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