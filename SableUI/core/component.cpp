#include <SableUI/core/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/styles/theme.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/utils.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

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

	for (BaseComponent* garbage : m_garbageChildren)
		SB_delete(garbage);

	m_garbageChildren.clear();
}

static void RebuildHoverListRecursive(SableUI::Element* el, std::vector<SableUI::Element*>& list)
{
	if (el->info.appearance.hasHoverBg)
		list.push_back(el);

	for (SableUI::Child* child : el->children)
	{
		if (child->type == SableUI::ChildType::ELEMENT)
		{
			RebuildHoverListRecursive(child->element, list);
		}
	}
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
	{
		for (size_t i = m_childCount; i < m_componentChildren.size(); i++)
			if (m_componentChildren[i])
				m_garbageChildren.push_back(m_componentChildren[i]);
	}

	m_componentChildren.resize(m_childCount);
}

void SableUI::BaseComponent::BackendInitialisePanel()
{
	if (rootElement) SB_delete(rootElement);

	if (!m_renderer)
		SableUI_Runtime_Error("Renderer has not been initialised for component");

	const Theme& t = GetTheme();

	ElementInfo info{};
	info.type = ElementType::Div;
	info.appearance.bg = t.base;
	info.layout.wType = RectType::Fill;
	info.layout.hType = RectType::Fill;
	rootElement = SB_new<Element>(m_renderer, info);
	rootElement->m_owner = this;

	SetCurrentComponent(this);
	SetElementBuilderContext(m_renderer, rootElement, false);
	LayoutWrapper();

	rootElement->LayoutChildren();
}

void SableUI::BaseComponent::BackendInitialiseFloatingPanel(const Rect& rect, const ElementInfo& p_info)
{
	if (rootElement) SB_delete(rootElement);

	if (!m_renderer)
		SableUI_Runtime_Error("Renderer has not been initialised for component");

	const Theme& t = GetTheme();

	ElementInfo info = p_info;
	info.type = ElementType::Div;
	if (info.appearance.bg == Colour{ 0, 0, 0, 0 })
		info.appearance.bg = t.base;
	info.layout.width = rect.w;
	info.layout.height = rect.h;
	info.layout.wType = RectType::Fixed;
	info.layout.hType = RectType::Fixed;
	rootElement = SB_new<Element>(m_renderer, info);
	rootElement->SetRect(rect);
	rootElement->m_owner = this;

	SetCurrentComponent(this);
	SetElementBuilderContext(m_renderer, rootElement, false);
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

void SableUI::BaseComponent::Render(int z)
{
	rootElement->Render(z);
}

void SableUI::BaseComponent::BackendInitialiseChild(const std::string& name, BaseComponent* parent, const ElementInfo& info)
{
	int n = parent->GetNumChildren();

	m_hash = GetHash(n, name.c_str());

	for (BaseComponent* c : parent->m_garbageChildren)
		if (c->m_hash == m_hash)
			CopyStateFrom(*c);

	m_renderer = parent->m_renderer;

	SetCurrentComponent(parent);
	StartDiv(info, this);
	LayoutWrapper();
	EndDiv();
}

SableUI::Element* SableUI::BaseComponent::GetRootElement()
{
	if (rootElement == nullptr)
	{
		SableUI_Runtime_Error("Attempt to access rootElement before is was initialised. Are you calling GetRootElement() inside of Layout()? -> this is unsupported, wrap your layout logic inside another div instead.");
	}
	return rootElement;
}

bool SableUI::BaseComponent::Rerender(bool* hasContentsChanged)
{
	Rect oldRect = { rootElement->rect };

	m_hoverElements.clear();

	// Generate virtual tree
	SetCurrentComponent(this);
	SetElementBuilderContext(m_renderer, rootElement, true);
	LayoutWrapper();
	VirtualNode* virtualRoot = SableUI::GetVirtualRootNode();

	if (rootElement->Reconcile(virtualRoot) && hasContentsChanged)
		*hasContentsChanged = true;

	for (BaseComponent* garbage : m_garbageChildren)
		SB_delete(garbage);

	m_garbageChildren.clear();

	m_hoverElements.clear();
	RebuildHoverListRecursive(rootElement, m_hoverElements);

	rootElement->LayoutChildren();
	rootElement->LayoutChildren();

	for (Element* el : m_hoverElements)
	{
		if (el->info.appearance.hasHoverBg)
		{
			bool res = RectBoundingBox(el->rect, m_lastEventCtx.mousePos);

			el->isHovered = res;
			el->wasHovered = false;

			if (res)
			{
				el->info.appearance.bg = el->info.appearance.hoverBg;
			}
			else
			{
				el->info.appearance.bg = el->originalBg;
			}

			el->SetRect(el->rect);
		}
	}

	Rect newRect = rootElement->rect;
	if (oldRect.w != newRect.w || oldRect.h != newRect.h)
		return true;

	Render();

	needsRerender = false;
	return false;
}

void SableUI::BaseComponent::HandleInput(const UIEventContext& ctx)
{
	// Propagate to element tree
	rootElement->DistributeInputToElements(ctx);

	m_lastEventCtx = ctx;
	OnUpdate(ctx);
	UpdateHoverStyling(ctx);
}

bool SableUI::BaseComponent::CheckAndUpdate()
{
	if (!needsRerender)
	{
		bool childChanged = rootElement->CheckElementTreeForChanges();
		return childChanged;
	}

	Rerender(nullptr);
	needsRerender = false;

	return true;
}

void SableUI::BaseComponent::PostLayoutUpdate(const UIEventContext& ctx)
{
	for (auto* child : m_componentChildren)
		child->PostLayoutUpdate(ctx);

	OnUpdatePostLayout(ctx);
}

void SableUI::BaseComponent::MarkDirty()
{
	needsRerender = true;
	PostEmptyEvent();
}

void SableUI::BaseComponent::CopyStateFrom(const BaseComponent& other)
{
	/* Ensure both components have the same number of states,
	 * Since components are defined at compile time it should always match, 
	 * but sanity check >> */
	if (m_states.size() != other.m_states.size())
	{
		SableUI_Warn("State count mismatch in CopyStateFrom (this: %zu, other: %zu)",
			m_states.size(), other.m_states.size());
	}

	size_t count = std::min(m_states.size(), other.m_states.size());
	for (size_t i = 0; i < count; i++)
	{
		// Polymorphic call to State<T>::Sync or Ref<T>::Sync
		m_states[i]->Sync(other.m_states[i]);
	}
}

SableUI::Element* SableUI::BaseComponent::GetElementById(const SableString& id)
{
	if (!rootElement)
	{
		SableUI_Warn("GetElementById() returned nullptr for ID: %s" , std::string(id).c_str());
		return nullptr;
	}

	return rootElement->GetElementById(id);
}

void SableUI::BaseComponent::RegisterHoverElement(Element* el)
{
	if (el->info.appearance.hasHoverBg)
		m_hoverElements.push_back(el);
}

void SableUI::BaseComponent::UpdateHoverStyling(const UIEventContext& ctx)
{
	for (Element* el : m_hoverElements)
	{
		el->wasHovered = el->isHovered;
		el->isHovered = RectBoundingBox(el->rect, ctx.mousePos);

		if (el->isHovered != el->wasHovered)
		{
			if (el->isHovered)
				el->info.appearance.bg = el->info.appearance.hoverBg;
			else
				el->info.appearance.bg = el->originalBg;

			el->SetRect(el->rect);
			MarkDirty();
		}
	}
}

SableUI::BaseComponent* SableUI::BaseComponent::AttachComponent(BaseComponent* component)
{
	if (!component)
		return nullptr;

	if (static_cast<size_t>(m_childCount) < m_componentChildren.size())
	{
		BaseComponent* existing = m_componentChildren[m_childCount];
		if (existing)
			m_garbageChildren.push_back(existing);

		m_componentChildren[m_childCount] = component;
	}
	else
	{
		m_componentChildren.push_back(component);
	}

	m_childCount++;
	return component;
}

void SableUI::_priv_comp_PostEmptyEvent()
{
	SableUI::PostEmptyEvent();
}
