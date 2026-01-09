#include <SableUI/core/floating_panel.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/core/panel.h>
#include <SableUI/core/renderer.h>
#include <SableUI/utils/utils.h>

using namespace SableMemory;

static int s_panelCount = 0;
SableUI::FloatingPanel::FloatingPanel(RendererBackend* renderer, const Rect& p_rect) : BasePanel(nullptr, renderer)
{
	s_panelCount++;
	type = PanelType::Floating;
	rect = p_rect;
}

SableUI::FloatingPanel::~FloatingPanel()
{
	SB_delete(m_component);
	s_panelCount--;
}

int SableUI::FloatingPanel::GetNumInstances()
{
	return s_panelCount;
}

SableUI::SplitterPanel* SableUI::FloatingPanel::AddSplitter(PanelType type)
{
	SableUI_Error("Base node cannot have any children, skipping call");
	return nullptr;
}

SableUI::ContentPanel* SableUI::FloatingPanel::AddPanel()
{
	SableUI_Error("Base node cannot have any children, skipping call");
	return nullptr;
}

void SableUI::FloatingPanel::Update()
{
	if (m_component == nullptr)
	{
		m_component = SB_new<BaseComponent>();
		m_component->SetRenderer(m_renderer);
		m_component->BackendInitialisePanel();
	}

	m_component->GetRootElement()->SetRect(rect);
	m_component->GetRootElement()->LayoutChildren();
	Render();
}

void SableUI::FloatingPanel::Render()
{
	if (m_component)
		m_component->Render();
}

void SableUI::FloatingPanel::DistributeEvents(const UIEventContext& ctx)
{
	if (m_component)
		m_component->HandleInput(ctx);
}

bool SableUI::FloatingPanel::UpdateComponents()
{
	if (!m_component)
		return false;

	bool changed = m_component->CheckAndUpdate();

	if (changed)
	{
		m_component->GetRootElement()->LayoutChildren();
		Update();
	}

	return changed;
}

void SableUI::FloatingPanel::PostLayoutUpdate(const UIEventContext& ctx)
{
	if (m_component)
		m_component->PostLayoutUpdate(ctx);
}

SableUI::Element* SableUI::FloatingPanel::GetElementById(const SableString& id)
{
	if (!m_component)
		return nullptr;

	return m_component->GetElementById(id);
}
