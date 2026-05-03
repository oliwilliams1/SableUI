#include <SableUI/core/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/styles/theme.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/drawable.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/states/state_base.h>
#include <SableUI/types/floating_panel_types.h>
#include <SableUI/core/window.h>
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

	if (m_rootElement) SB_delete(m_rootElement);

	for (BaseComponent* child : m_componentChildren)
		if (child) SB_delete(child);

	m_componentChildren.clear();

	for (BaseComponent* garbage : m_garbageChildren)
		SB_delete(garbage);

	m_garbageChildren.clear();
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
	if (m_rootElement) SB_delete(m_rootElement);

	if (!m_renderer)
		SableUI_Runtime_Error("Renderer has not been initialised for component");

	const Theme& t = GetTheme();

	ElementInfo info{};
	info.type = ElementType::Div;
	info.appearance.bg = t.base;
	info.layout.wType = RectType::Fill;
	info.layout.hType = RectType::Fill;
	m_rootElement = SB_new<Element>(m_renderer, info);
	m_rootElement->m_owner = this;

	SetCurrentComponent(this);
	SetElementBuilderContext(m_renderer, m_rootElement, false);
	LayoutWrapper();

	m_rootElement->LayoutChildren(_getCurrentContext()->GetMainCommandBuffer());
}

void SableUI::BaseComponent::BackendInitialiseFloatingPanel(const Rect& rect, const ElementInfo& p_info)
{
	if (m_rootElement) SB_delete(m_rootElement);

	if (!m_renderer)
		SableUI_Runtime_Error("Renderer has not been initialised for component");

	CommandBuffer& cmd = _getCurrentContext()->GetMainCommandBuffer();

	ElementInfo info = p_info;
	info.type = ElementType::Div;

	info.layout.width = rect.w;
	info.layout.height = rect.h;
	info.layout.wType = RectType::Fixed;
	info.layout.hType = RectType::Fixed;
	info.appearance.bg = Colour(0, 0, 0, 0);
	m_rootElement = SB_new<Element>(m_renderer, info);
	m_rootElement->SetRect(cmd, rect);
	m_rootElement->m_owner = this;

	SetCurrentComponent(this);
	SetElementBuilderContext(m_renderer, m_rootElement, false);
	LayoutWrapper();

	m_rootElement->LayoutChildren(cmd);
}

void SableUI::BaseComponent::SetRenderer(RendererBackend* renderer)
{
	m_renderer = renderer;
}

SableUI::RendererBackend* SableUI::BaseComponent::GetRenderer()
{
	return m_renderer;
}

static size_t GetHash(int n, const char* name)
{
	size_t h = 0;

	for (int i = 0; i < strlen(name); i++)
		h = (h << (5 + n)) - h + name[i];

	h ^= (n * 0x9e3779b) ^ (n << 15);

	return h;
}

void SableUI::BaseComponent::Render(const DrawableDrawData& drData, int z)
{
	m_rootElement->Render(drData, z);
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
	if (m_rootElement == nullptr)
	{
		SableUI_Runtime_Error("Attempt to access m_rootElement before is was initialised. Are you calling GetRootElement() inside of Layout()? -> this is unsupported, wrap your layout logic inside another div instead.");
	}
	return m_rootElement;
}

void SableUI::BaseComponent::SetRootElement(Element* element)
{
	m_rootElement = element;
}

int SableUI::BaseComponent::GetNumChildren() const
{
	return m_childCount;
}

bool SableUI::BaseComponent::Rerender(const DrawableDrawData& drData, bool* hasContentsChanged)
{
	Rect oldRect = { m_rootElement->rect };

	// Generate virtual tree
	SetCurrentComponent(this);
	SetElementBuilderContext(m_renderer, m_rootElement, true);
	LayoutWrapper();
	VirtualNode* virtualRoot = SableUI::GetVirtualRootNode();

	if (m_rootElement->Reconcile(virtualRoot) && hasContentsChanged)
		*hasContentsChanged = true;

	for (BaseComponent* garbage : m_garbageChildren)
		SB_delete(garbage);

	m_garbageChildren.clear();

	m_rootElement->LayoutChildren(drData.cmd);

	Rect newRect = m_rootElement->rect;
	if (oldRect.w != newRect.w || oldRect.h != newRect.h)
		return true;

	Render(drData);

	needsRerender = false;
	return false;
}

void SableUI::BaseComponent::HandleInput(const UIEventContext& ctx, int z)
{
	m_rootElement->DistributeInputToElements(ctx, z);

	for (FloatingPanelBase* panel : m_floatingPanels)
		if (panel->IsOpen())
			panel->HandleInput(ctx, panel->GetZIndex());

	m_lastEventCtx = ctx;
	OnUpdate(ctx);
}

bool SableUI::BaseComponent::CheckAndUpdate(const DrawableDrawData& drData)
{
	bool fpChanged = false;
	for (FloatingPanelBase* panel : m_floatingPanels)
		if (panel->IsOpen())
			fpChanged |= panel->CheckAndUpdate(drData);

	if (!needsRerender)
	{
		bool childChanged = m_rootElement->CheckElementTreeForChanges(drData);
		return childChanged || fpChanged;
	}

	Rerender(drData, nullptr);
	needsRerender = false;

	return true;
}

void SableUI::BaseComponent::PostLayoutUpdate(const UIEventContext& ctx)
{
	for (auto* child : m_componentChildren)
		child->PostLayoutUpdate(ctx);

	for (FloatingPanelBase* panel : m_floatingPanels)
		if (panel->IsOpen())
			panel->PostLayoutUpdate(ctx);

	OnUpdatePostLayout(ctx);
}

void SableUI::BaseComponent::RegisterState(StateBase* state)
{
	m_states.push_back(state);
}

void SableUI::BaseComponent::RegisterFloatingPanel(FloatingPanelBase* state)
{
	m_floatingPanels.push_back(state);
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
		m_states[i]->Sync(other.m_states[i]);
	}
}

SableUI::Element* SableUI::BaseComponent::GetElementById(const SableString& id)
{
	if (!m_rootElement)
	{
		SableUI_Warn("GetElementById() returned nullptr for ID: %s", std::string(id).c_str());
		return nullptr;
	}

	return m_rootElement->GetElementById(id);
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