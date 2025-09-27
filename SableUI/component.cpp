#include "SableUI/component.h"
#include "SableUI/SableUI.h"

SableUI::BaseComponent::BaseComponent(Colour colour)
{
	m_bgColour = colour;
}

SableUI::BaseComponent::~BaseComponent()
{
	if (rootElement) delete rootElement;
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

	StartDiv(info, this);
	Layout();
	EndDiv();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	return rootElement;
}

bool SableUI::BaseComponent::Rerender()
{
	Rect oldRect = { rootElement->rect };

	for (Child* child : rootElement->children) delete child;
	rootElement->children.clear();

	SetElementBuilderContext(m_renderer, rootElement);
	Layout();

	rootElement->LayoutChildren();
	rootElement->LayoutChildren(); // dirty fix

	Rect newRect = { rootElement->rect };

	if (oldRect.w != rootElement->rect.w || oldRect.h != rootElement->rect.h)
		return true;

	needsRerender = false;

	rootElement->Render();

	return false;
}

bool SableUI::BaseComponent::comp_PropagateComponentStateChanges()
{
	bool res = rootElement->el_PropagateComponentStateChanges();

	bool needsFullRerender = false;
	if (needsRerender)
	{
		// Does the re-rendering cause the size of root to be changed?
		// If so, rerender from next significant component
		if (Rerender())
			needsFullRerender = true;
		else
			needsFullRerender = false;
	}


	if (res) needsRerender = true;

	return needsRerender;
}