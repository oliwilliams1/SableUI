#include "SableUI/component.h"
#include "SableUI/SableUI.h"
#include "SableUI/memory.h"
#include <cstring>
#include <SableUI/console.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/renderer.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>

using namespace SableMemory;

static int s_numComponents = 0;
SableUI::BaseComponent::BaseComponent(Colour colour)
{
	s_numComponents++;
	m_bgColour = colour;
}

SableUI::BaseComponent::~BaseComponent()
{
	s_numComponents--;

	if (rootElement) SB_delete(rootElement);

	for (BaseComponent* child : m_componentChildren)
		if (child) SB_delete(child);
	
	m_componentChildren.clear();

	for (CustomTargetQueue** queuePtr : m_customTargetQueuePtrs)
	{
		if (queuePtr && *queuePtr)
		{
			SB_delete(*queuePtr);
			*queuePtr = nullptr;
		}
	}

	m_customTargetQueuePtrs.clear();
}

int SableUI::BaseComponent::GetNumInstances()
{
	return s_numComponents;
}

void SableUI::BaseComponent::LayoutWrapper()
{
	m_childCount = 0;
	Layout();

	if (m_componentChildren.size() > static_cast<size_t>(m_childCount))
		for (size_t i = m_childCount; i < m_componentChildren.size(); i++)
			if (m_componentChildren[i])
				SB_delete(m_componentChildren[i]);

	m_componentChildren.resize(m_childCount);
}

void SableUI::BaseComponent::BackendInitialisePanel(RendererBackend* renderer)
{
	if (rootElement) SB_delete(rootElement);

	m_renderer = renderer;

	rootElement = SB_new<Element>(renderer, ElementType::DIV);
	rootElement->setBgColour(m_bgColour);
	rootElement->setWType(RectType::FILL);
	rootElement->setHType(RectType::FILL);

	SetElementBuilderContext(renderer, rootElement, false);
	LayoutWrapper();

	rootElement->LayoutChildren();
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
	size_t minSize = std::min(m_stateBlocks.size(), other.m_stateBlocks.size());

	if (m_stateBlocks.size() != other.m_stateBlocks.size())
	{
		SableUI_Warn("Component state block count mismatch (this: %zu, other: %zu). Copying %zu blocks.",
			m_stateBlocks.size(), other.m_stateBlocks.size(), minSize);
	}

	for (size_t i = 0; i < minSize; i++)
	{
		m_stateBlocks[i].CopyFrom(other.m_stateBlocks[i]);
	}
}

SableUI::Element* SableUI::BaseComponent::GetElementById(const SableString& id)
{
	if (!rootElement)
		return nullptr;

	return rootElement->GetElementById(id);
}