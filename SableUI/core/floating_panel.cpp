#include <SableUI/core/floating_panel.h>
#include <SableUI/core/component.h>
#include <SableUI/core/component_registry.h>
#include <SableUI/core/element.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/events.h>
#include <SableUI/core/window.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/console.h>
#include <algorithm>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "FloatingPanel"

using namespace SableUI;
using namespace SableMemory;

// ============================================================================
// FloatingPanel
// ============================================================================
FloatingPanel::FloatingPanel(const std::string& id, RendererBackend* renderer, const FloatingPanelInfo& info)
	: m_id(id), m_renderer(renderer), m_bounds(info.bounds), m_zIndex(info.zIndex), m_modal(info.modal) {}

FloatingPanel::~FloatingPanel()
{
	if (m_component)
		SB_delete(m_component);
}

void FloatingPanel::AttachComponent(BaseComponent* component)
{
	if (m_component)
		SB_delete(m_component);

	m_component = component;
	m_component->SetRenderer(m_renderer);
	m_component->BackendInitialisePanel();
	Update();
}

void FloatingPanel::Update()
{
	if (!m_component) return;

	m_component->GetRootElement()->SetRect(m_bounds);
	m_component->GetRootElement()->LayoutChildren();
	m_component->GetRootElement()->LayoutChildren();
}

void FloatingPanel::Render()
{
	if (!m_component) return;
	m_component->Render();
}

void FloatingPanel::HandleInput(const UIEventContext& ctx, bool& handled)
{
	if (!m_component) return;

	if (!IsPointInside(ctx.mousePos))
		return;

	m_component->HandleInput(ctx);
	handled = true;
}

bool FloatingPanel::IsPointInside(ivec2 point) const
{
	return RectBoundingBox(m_bounds, point);
}

void FloatingPanel::SetBounds(const Rect& bounds)
{
	m_bounds = bounds;
	Update();
}

// ============================================================================
// FloatingPanelManager
// ============================================================================
FloatingPanelManager::FloatingPanelManager(RendererBackend* renderer)
	: m_renderer(renderer)
{
}

FloatingPanelManager::~FloatingPanelManager()
{
	m_panels.clear();
}

FloatingPanel* FloatingPanelManager::OpenPanel(const std::string& componentName, const FloatingPanelInfo& info)
{
	if (IsPanelActive(info.id))
	{
		SableUI_Warn("Panel with ID '%s' already exists", info.id.c_str());
		return GetPanel(info.id);
	}

	FloatingPanel* panel = SB_new<FloatingPanel>(info.id, m_renderer, info);

	BaseComponent* component = ComponentRegistry::GetInstance().Create(componentName);
	if (!component)
	{
		SableUI_Error("Failed to create component '%s' for floating panel", componentName.c_str());
		return nullptr;
	}

	panel->AttachComponent(component);

	m_panels.push_back(panel);

	SortByZIndex();

	SableUI_Log("Opened floating panel '%s' with component '%s'", info.id.c_str(), componentName.c_str());

	return panel;
}

void FloatingPanelManager::ClosePanel(const std::string& id)
{
	auto it = std::find_if(m_panels.begin(), m_panels.end(),
		[&id](const FloatingPanel* panel) {
			return panel->GetID() == id;
		});

	if (it != m_panels.end())
	{
		SableUI_Log("Closed floating panel '%s'", id.c_str());
		m_panels.erase(it);
	}
}

bool FloatingPanelManager::IsPanelActive(const std::string& id) const
{
	return std::any_of(m_panels.begin(), m_panels.end(),
		[&id](const FloatingPanel* panel) {
			return panel->GetID() == id;
		});
}

FloatingPanel* FloatingPanelManager::GetPanel(const std::string& id) const
{
	auto it = std::find_if(m_panels.begin(), m_panels.end(),
		[&id](const FloatingPanel* panel) {
			return panel->GetID() == id;
		});

	return (it != m_panels.end()) ? *it : nullptr;
}

void FloatingPanelManager::UpdateAll()
{
	for (FloatingPanel* panel : m_panels)
	{
		if (panel->GetComponent() && panel->GetComponent()->CheckAndUpdate())
		{
			panel->Update();
		}
	}
}

void FloatingPanelManager::RenderAll()
{
	for (FloatingPanel* panel : m_panels)
	{
		panel->Render();
	}
}

void FloatingPanelManager::HandleInput(const UIEventContext& ctx, bool& blockMainContent)
{
	bool inputHandled = false;

	for (auto it = m_panels.rbegin(); it != m_panels.rend(); ++it)
	{
		FloatingPanel* panel = *it;

		if (!inputHandled)
		{
			panel->HandleInput(ctx, inputHandled);

			if (inputHandled && panel->IsModal())
			{
				blockMainContent = true;
				break;
			}
		}

		if (panel->GetComponent())
			panel->GetComponent()->PostLayoutUpdate(ctx);
	}

	if (inputHandled)
	{
		for (FloatingPanel* panel : m_panels)
		{
			if (panel->IsModal())
			{
				blockMainContent = true;
				break;
			}
		}
	}
}

void FloatingPanelManager::BringToFront(const std::string& id)
{
	auto it = std::find_if(m_panels.begin(), m_panels.end(),
		[&id](const FloatingPanel* panel) {
			return panel->GetID() == id;
		});

	if (it != m_panels.end())
	{
		int maxZ = 100;
		for (const FloatingPanel* panel : m_panels)
		{
			maxZ = std::max(maxZ, panel->GetZIndex());
		}

		(*it)->SetZIndex(maxZ + 1);
		SortByZIndex();
	}
}

void FloatingPanelManager::SortByZIndex()
{
	std::sort(m_panels.begin(), m_panels.end(),
		[](const FloatingPanel* a, const FloatingPanel* b) {
			return a->GetZIndex() < b->GetZIndex();
		});
}

namespace SableUI::FloatingPanels
{
	FloatingPanel* Open(const std::string& componentName, const FloatingPanelInfo& info)
	{
		Window* ctx = GetContext();
		if (!ctx)
		{
			SableUI_Error("No context set. Call SetContext() first");
			return nullptr;
		}

		return ctx->GetFloatingPanelManager().OpenPanel(componentName, info);
	}

	void Close(const std::string& id)
	{
		Window* ctx = GetContext();
		if (!ctx)
		{
			SableUI_Error("No context set. Call SetContext() first");
			return;
		}

		ctx->GetFloatingPanelManager().ClosePanel(id);
	}

	bool IsActive(const std::string& id)
	{
		Window* ctx = GetContext();
		if (!ctx)
		{
			SableUI_Error("No context set. Call SetContext() first");
			return false;
		}

		return ctx->GetFloatingPanelManager().IsPanelActive(id);
	}

	FloatingPanel* Get(const std::string& id)
	{
		Window* ctx = GetContext();
		if (!ctx)
		{
			SableUI_Error("No context set. Call SetContext() first");
			return nullptr;
		}

		return ctx->GetFloatingPanelManager().GetPanel(id);
	}

	void BringToFront(const std::string& id)
	{
		Window* ctx = GetContext();
		if (!ctx)
		{
			SableUI_Error("No context set. Call SetContext() first");
			return;
		}

		ctx->GetFloatingPanelManager().BringToFront(id);
	}
}