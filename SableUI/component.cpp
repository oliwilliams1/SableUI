#include "SableUI/component.h"
#include "SableUI/SableUI.h"

static int s_ctr = 0;
SableUI::BaseComponent::BaseComponent(Colour colour)
{
	s_ctr++;
	m_bgColour = colour;
}

SableUI::BaseComponent::~BaseComponent()
{
	s_ctr--;
	if (rootElement) delete rootElement;
}

void SableUI::BaseComponent::BackendInitialisePanel(Renderer* renderer)
{
	if (rootElement) delete rootElement;

	m_renderer = renderer;

	rootElement = new Element(renderer, ElementType::DIV);
	rootElement->Init(renderer, ElementType::DIV);
	rootElement->setBgColour(m_bgColour);

	SetElementBuilderContext(renderer, rootElement, false);
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

	// Generate virual tree
	SableUI::SetElementBuilderContext(m_renderer, rootElement, true);
	Layout();
	VirtualNode* virtualRoot = SableUI::GetVirtualRootNode();

	rootElement->Reconcile(virtualRoot);

	rootElement->LayoutChildren();
	rootElement->LayoutChildren();

	Rect newRect = rootElement->rect;
	if (oldRect.w != newRect.w || oldRect.h != newRect.h)
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