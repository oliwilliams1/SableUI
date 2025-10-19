#include "SableUI/component.h"
#include "SableUI/SableUI.h"
#include "SableUI/memory.h"

using namespace SableMemory;

static int s_ctr = 0;
SableUI::BaseComponent::BaseComponent(Colour colour)
{
	s_ctr++;
	m_bgColour = colour;
}

SableUI::BaseComponent::~BaseComponent()
{
	s_ctr--;

	if (rootElement) SB_delete(rootElement);
	m_componentChildren.clear();
}

void SableUI::BaseComponent::LayoutWrapper()
{
	m_childCount = 0;
	Layout();

	m_componentChildren.resize(m_childCount);
}

void SableUI::BaseComponent::BackendInitialisePanel(Renderer* renderer)
{
	if (rootElement) SB_delete(rootElement);

	m_renderer = renderer;

	rootElement = SB_new<Element>(renderer, ElementType::DIV);
	rootElement->Init(renderer, ElementType::DIV);
	rootElement->setBgColour(m_bgColour);

	SetElementBuilderContext(renderer, rootElement, false);
	LayoutWrapper();
}

static size_t GetHash(int n, const char* name)
{
	size_t h = 0;

	for (int i = 0; i < strlen(name); i++)
		h = (h << (5 + n)) - h + name[i];

	h ^= (n * 0x9e3779b) ^ (n << 15);

	return h;
}

void SableUI::BaseComponent::BackendInitialiseChild(const char* name, BaseComponent* parent, const ElementInfo& info)
{
	if (rootElement) SB_delete(rootElement);

	int n = parent->GetNumChildren();

	m_hash = GetHash(n, name);

	for (int c = 0; c < n; c++)
	{
		BaseComponent* child = parent->m_componentChildren[c];

		if (child->m_hash == m_hash)
			CopyStateFrom(*child);
	}

	m_renderer = parent->m_renderer;

	StartDiv(info, this);
	LayoutWrapper();
	EndDiv();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	return rootElement;
}

bool SableUI::BaseComponent::Rerender(bool* hasContentsChanged)
{
	Rect oldRect = { rootElement->rect };

	// Generate virtual tree
	SetElementBuilderContext(m_renderer, rootElement, true);
	LayoutWrapper();
	VirtualNode* virtualRoot = SableUI::GetVirtualRootNode();

	if (rootElement->Reconcile(virtualRoot) && hasContentsChanged)
		*hasContentsChanged = true;

	rootElement->LayoutChildren();
	rootElement->LayoutChildren();

	Rect newRect = rootElement->rect;
	if (oldRect.w != newRect.w || oldRect.h != newRect.h)
		return true;

	rootElement->Render();

	needsRerender = false;
	return false;
}


void SableUI::BaseComponent::comp_PropagateEvents(const UIEventContext& ctx)
{
	OnUpdate(ctx);

	rootElement->el_PropagateEvents(ctx);
}

bool SableUI::BaseComponent::comp_PropagateComponentStateChanges(bool* hasContentsChanged)
{
	bool res = rootElement->el_PropagateComponentStateChanges(hasContentsChanged);

	if (needsRerender)
	{
		if (hasContentsChanged) *hasContentsChanged = true;
		// Does the re-rendering cause the size of root to be changed?
		// If so, rerender from next significant component
		if (Rerender(hasContentsChanged))
			needsRerender = true;
		else
			needsRerender = false;
	}

	if (needsRerender)
	{
		needsRerender = false;
		return true;
	}

	return false;
}

void SableUI::BaseComponent::CopyStateFrom(const BaseComponent& other)
{
	if (m_stateBlocks.size() != other.m_stateBlocks.size())
	{
		SableUI_Runtime_Error("Trying to copy state from component with different number of state variables");
		return;
	}

	for (size_t i = 0; i < m_stateBlocks.size(); i++)
	{
		m_stateBlocks[i].CopyFrom(other.m_stateBlocks[i]);
	}
}