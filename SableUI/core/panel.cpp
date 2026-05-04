#include <SableUI/core/panel.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/console.h>
#include <SableUI/styles/theme.h>
#include <algorithm>
#include <vector>
#include <SableUI/core/window.h>

using namespace SableMemory;

static int s_basePanelCount = 0;
SableUI::BasePanel::BasePanel(BasePanel* parent, RendererBackend* renderer) : parent(parent), m_renderer(renderer)
{
	type = PanelType::Base;
	wType = RectType::Fill;
	hType = RectType::Fill;
	
	s_basePanelCount++;
}

SableUI::BasePanel::~BasePanel()
{
	s_basePanelCount--;
}

int SableUI::BasePanel::GetNumInstances()
{
	return s_basePanelCount;
}

void SableUI::BasePanel::DistributeEvents(const UIInputState& ctx, int z)
{
	for (BasePanel* child : children)
		child->DistributeEvents(ctx, z);
}

bool SableUI::BasePanel::UpdateComponents(const DrawableDrawData& drData)
{
	bool anyChanged = false;
	for (BasePanel* child : children)
		anyChanged |= child->UpdateComponents(drData);
	return anyChanged;
}

void SableUI::BasePanel::PostLayoutUpdate(const UIInputState& ctx, int z)
{
	for (BasePanel* child : children)
		child->PostLayoutUpdate(ctx, z);
}

SableUI::Element* SableUI::BasePanel::GetElementById(const SableString& id)
{
	for (BasePanel* child : children)
	{
		Element* found = child->GetElementById(id);
		if (found) return found;
	}

	return nullptr;
}

// ============================================================================
// Root Panel
// ============================================================================
static int s_rootPanelCount = 0;
SableUI::RootPanel::RootPanel(RendererBackend* renderer, int w, int h) : BasePanel(nullptr, renderer)
{
	s_rootPanelCount++;
	type = PanelType::Root;
	rect.w = w;
	rect.h = h;
}

int SableUI::RootPanel::GetNumInstances()
{
	return s_rootPanelCount;
}

SableUI::RootPanel::~RootPanel()
{
	s_rootPanelCount--;
	for (SableUI::BasePanel* child : children) SB_delete(child);
	children.clear();
}

void SableUI::RootPanel::Render(const DrawableDrawData& drData)
{
	for (SableUI::BasePanel* child : children)
		child->Render(drData);
}

void SableUI::RootPanel::Recalculate(const DrawableDrawData& drData)
{
	for (SableUI::BasePanel* child : children)
	{
		child->rect.x = rect.x;
		child->rect.y = rect.y;
		child->rect.w = rect.w;
		child->rect.h = rect.h;

		child->CalculateScales();
		child->CalculatePositions(drData);
		child->CalculateMinBounds();

		if (child->type == PanelType::Base)
		{
			if (SableUI::ContentPanel* panelChild = dynamic_cast<SableUI::ContentPanel*>(child))
			{
				panelChild->Update(drData);
			}
			else
			{
				SableUI_Error("Root child marked as Base but not a Panel instance.");
			}
		}
	}
}

SableUI::SplitterPanel* SableUI::RootPanel::AddSplitter(CommandBuffer& cmd, PanelType type)
{
	if (children.size() > 0)
	{
		SableUI_Error("Root node cannot have more than one child, dismissing call");
		return nullptr;
	}
	SplitterPanel* node = SB_new<SplitterPanel>(this, type, m_renderer);
	children.push_back(node);

	const Window* window = _getCurrentContext();
	DrawableDrawData drData = DrawableDrawData(
		cmd,
		window->GetSurface(),
		{ 0, 0, window->m_windowSize.x, window->m_windowSize.y },
		GetContextResources(m_renderer)
	);

	Recalculate(drData);
	return node;
}

SableUI::ContentPanel* SableUI::RootPanel::AddPanel(CommandBuffer& cmd)
{
	if (children.size() > 0)
	{
		SableUI_Error("Root node cannot have more than one child, dismissing call");
		return nullptr;
	}
	ContentPanel* node = SB_new<ContentPanel>(this, m_renderer);
	children.push_back(node);

	const Window* window = _getCurrentContext();
	DrawableDrawData drData = DrawableDrawData(
		cmd,
		window->GetSurface(),
		{ 0, 0, window->m_windowSize.x, window->m_windowSize.y },
		GetContextResources(m_renderer)
	);

	Recalculate(drData);
	return node;
}

SableUI::BasePanel* SableUI::BasePanel::FindRoot()
{
	BasePanel* node = this;
	while (node->parent != nullptr) node = node->parent;
	return node;
}

void SableUI::RootPanel::CalculateScales()
{
	SableUI_Error("Method does not exist for root node, use RootNode.Recalculate() instead");
	return;
}

void SableUI::RootPanel::CalculatePositions(const DrawableDrawData& drData)
{
	SableUI_Error("Method does not exist for root node, use RootNode.Recalculate() instead");
	return;
}

void SableUI::RootPanel::Resize(int w, int h)
{
	rect.w = w;
	rect.h = h;
}

// ============================================================================
// Splitter Panel
// ============================================================================
static int s_splitterPanelCount = 0;
SableUI::SplitterPanel::SplitterPanel(BasePanel* parent, PanelType type, RendererBackend* renderer)
	: BasePanel(parent, renderer)
{
	m_drawable = SableMemory::SB_new<DrawableSplitter>();
	s_splitterPanelCount++;
	this->type = type;
}

int SableUI::SplitterPanel::GetNumInstances()
{
	return s_splitterPanelCount;
}

void SableUI::SplitterPanel::Render(const DrawableDrawData& drData)
{
	if (!m_drawableUpToDate) Update(drData);
	for (SableUI::BasePanel* child : children)
		child->Render(drData);

	m_drawable->m_zIndex = 999;
	m_drawable->RecordCommands(drData);
}

SableUI::SplitterPanel* SableUI::SplitterPanel::AddSplitter(CommandBuffer& cmd, PanelType type)
{
	SplitterPanel* node = SB_new<SplitterPanel>(this, type, m_renderer);
	children.push_back(node);

	const Window* window = _getCurrentContext();
	DrawableDrawData drData = DrawableDrawData(
		cmd,
		window->GetSurface(),
		{ 0, 0, window->m_windowSize.x, window->m_windowSize.y },
		GetContextResources(m_renderer)
	);

	FindRoot()->Recalculate(drData);
	return node;
}

SableUI::ContentPanel* SableUI::SplitterPanel::AddPanel(CommandBuffer& cmd)
{
	ContentPanel* node = SB_new<ContentPanel>(this, m_renderer);
	children.push_back(node);

	const Window* window = _getCurrentContext();
	DrawableDrawData drData = DrawableDrawData(
		cmd,
		window->GetSurface(),
		{ 0, 0, window->m_windowSize.x, window->m_windowSize.y },
		GetContextResources(m_renderer)
	);

	FindRoot()->Recalculate(drData);
	return node;
}

void SableUI::SplitterPanel::CalculateScales()
{
	if (children.empty()) return;

	m_drawableUpToDate = false;

	if (type == PanelType::HorizontalSplitter)
	{
		int totalFixedWidth = 0;
		int numFillChildren = 0;
		std::vector<BasePanel*> fillChildren;

		for (BasePanel* child : children)
		{
			child->rect.h = rect.h;
			child->hType = RectType::Fill;

			if (child->wType == RectType::Fixed || child->wType == RectType::FitContent)
			{
				child->rect.w = (child->wType == RectType::Fixed) ? child->rect.w : std::max(0, child->minBounds.x);

				if (child->maxBounds.x > 0 && child->rect.w > child->maxBounds.x)
				{
					child->rect.w = child->maxBounds.x;
				}

				totalFixedWidth += child->rect.w;
			}
			else
			{
				fillChildren.push_back(child);
				numFillChildren++;
			}
		}

		int availableWidth = rect.w - totalFixedWidth;

		if (numFillChildren > 0)
		{
			int totalMinFillWidth = 0;
			for (BasePanel* child : fillChildren)
			{
				totalMinFillWidth += child->minBounds.x;
			}

			if (availableWidth >= totalMinFillWidth)
			{
				int remainingWidth = availableWidth - totalMinFillWidth;
				int widthPerFillChild = remainingWidth / numFillChildren;
				int leftoverWidth = remainingWidth % numFillChildren;

				for (size_t i = 0; i < fillChildren.size(); i++)
				{
					int calculatedWidth = fillChildren[i]->minBounds.x + widthPerFillChild + (i < leftoverWidth ? 1 : 0);

					if (fillChildren[i]->maxBounds.x > 0 && calculatedWidth > fillChildren[i]->maxBounds.x)
					{
						int excess = calculatedWidth - fillChildren[i]->maxBounds.x;
						fillChildren[i]->rect.w = fillChildren[i]->maxBounds.x;
						fillChildren[i]->wType = RectType::Fixed;

						if (i + 1 < fillChildren.size())
						{
							leftoverWidth += excess;
						}
					}
					else
					{
						fillChildren[i]->rect.w = calculatedWidth;
					}
				}
			}
			else
			{
				for (BasePanel* child : fillChildren)
				{
					child->rect.w = child->minBounds.x;
				}
			}
		}
	}
	else if (type == PanelType::VerticalSplitter)
	{
		int totalFixedHeight = 0;
		int numFillChildren = 0;
		std::vector<BasePanel*> fillChildren;

		for (BasePanel* child : children)
		{
			child->rect.w = rect.w;
			child->wType = RectType::Fill;

			if (child->hType == RectType::Fixed || child->hType == RectType::FitContent)
			{
				child->rect.h = (child->hType == RectType::Fixed) ? child->rect.h : std::max(0, child->minBounds.y);

				if (child->maxBounds.y > 0 && child->rect.h > child->maxBounds.y)
				{
					child->rect.h = child->maxBounds.y;
				}

				totalFixedHeight += child->rect.h;
			}
			else
			{
				fillChildren.push_back(child);
				numFillChildren++;
			}
		}

		int availableHeight = rect.h - totalFixedHeight;

		if (numFillChildren > 0)
		{
			int totalMinFillHeight = 0;
			for (BasePanel* child : fillChildren)
			{
				totalMinFillHeight += child->minBounds.y;
			}

			if (availableHeight >= totalMinFillHeight)
			{
				int remainingHeight = availableHeight - totalMinFillHeight;
				int heightPerFillChild = remainingHeight / numFillChildren;
				int leftoverHeight = remainingHeight % numFillChildren;

				for (size_t i = 0; i < fillChildren.size(); i++)
				{
					int calculatedHeight = fillChildren[i]->minBounds.y + heightPerFillChild + (i < leftoverHeight ? 1 : 0);

					if (fillChildren[i]->maxBounds.y > 0 && calculatedHeight > fillChildren[i]->maxBounds.y)
					{
						int excess = calculatedHeight - fillChildren[i]->maxBounds.y;
						fillChildren[i]->rect.h = fillChildren[i]->maxBounds.y;
						fillChildren[i]->hType = RectType::Fixed;

						if (i + 1 < fillChildren.size())
						{
							leftoverHeight += excess;
						}
					}
					else
					{
						fillChildren[i]->rect.h = calculatedHeight;
					}
				}
			}
			else
			{
				for (BasePanel* child : fillChildren)
				{
					child->rect.h = child->minBounds.y;
				}
			}
		}
	}

	for (BasePanel* child : children)
		child->CalculateScales();
}

void SableUI::SplitterPanel::CalculatePositions(const DrawableDrawData& drData)
{
	if (children.empty()) return;

	m_drawableUpToDate = false;
	bool toUpdate = false;

	vec2 cursor = { rect.x, rect.y };

	if (type == PanelType::HorizontalSplitter)
	{
		for (BasePanel* child : children)
		{
			vec2 originalPosition = { child->rect.x, child->rect.y };

			child->rect.x = cursor.x;
			child->rect.y = cursor.y;

			if (child->rect.x != originalPosition.x || child->rect.y != originalPosition.y)
			{
				toUpdate = true;
			}

			cursor.x += child->rect.w;
		}
	}
	else if (type == PanelType::VerticalSplitter)
	{
		for (BasePanel* child : children)
		{
			vec2 originalPosition = { child->rect.x, child->rect.y };

			child->rect.x = cursor.x;
			child->rect.y = cursor.y;

			if (child->rect.x != originalPosition.x || child->rect.y != originalPosition.y)
			{
				toUpdate = true;
			}

			cursor.y += child->rect.h;
		}
	}

	for (BasePanel* child : children)
	{
		bool childPositionChanged = false;
		child->CalculatePositions(drData);
		if (childPositionChanged)
			toUpdate = true;
	}

	if (toUpdate) Update(drData);
}

void SableUI::SplitterPanel::CalculateMinBounds()
{
	if (children.empty())
	{
		minBounds = { 20, 20 };
		return;
	}

	if (type == PanelType::HorizontalSplitter)
	{
		int totalWidth = 0;
		int maxHeight = 0;

		for (BasePanel* child : children)
		{
			child->CalculateMinBounds();
			totalWidth += child->wType == SableUI::Fill ? child->minBounds.x : child->rect.w;
			maxHeight = std::max(maxHeight, child->hType == SableUI::Fill ? child->minBounds.y : child->rect.h);
		}

		minBounds = { totalWidth, maxHeight };
	}
	else if (type == PanelType::VerticalSplitter)
	{
		int totalHeight = 0;
		int maxWidth = 0;

		for (BasePanel* child : children)
		{
			child->CalculateMinBounds();
			totalHeight += child->hType == SableUI::Fill ? child->minBounds.y : child->rect.h;
			maxWidth = std::max(maxWidth, child->wType == SableUI::Fill ? child->minBounds.x : child->rect.w);
		}

		minBounds = { maxWidth, totalHeight };
	}
}

void SableUI::SplitterPanel::Update(const DrawableDrawData& drData)
{
	std::vector<int> segments;

	for (SableUI::BasePanel* child : children)
	{
		child->Update(drData);

		if (type == PanelType::HorizontalSplitter)
			segments.push_back(child->rect.x - rect.x);
		if (type == PanelType::VerticalSplitter)
			segments.push_back(child->rect.y - rect.y);
	}
	
	const Theme& t = GetTheme();
	m_drawable->Update(rect, t.surface2, type, bSize, segments);
	m_drawableUpToDate = true;
	Render(drData);
}

SableUI::SplitterPanel::~SplitterPanel()
{
	for (BasePanel* child : children)
		SB_delete(child);
	children.clear();

	SableMemory::SB_delete(m_drawable);
	s_splitterPanelCount--;
}

// ============================================================================
// Content Panel
// ============================================================================
static int s_panelCount = 0; 
SableUI::ContentPanel::ContentPanel(BasePanel* parent, RendererBackend* renderer) : BasePanel(parent, renderer)
{
	s_panelCount++;
	type = PanelType::Base;
}

SableUI::ContentPanel::~ContentPanel()
{
	SB_delete(m_component);
	s_panelCount--;
}

int SableUI::ContentPanel::GetNumInstances()
{
	return s_panelCount;
}

SableUI::SplitterPanel* SableUI::ContentPanel::AddSplitter(CommandBuffer& cmd, PanelType type)
{
	SableUI_Error("Base node cannot have any children, skipping call");
	return nullptr;
}

SableUI::ContentPanel* SableUI::ContentPanel::AddPanel(CommandBuffer& cmd)
{
	SableUI_Error("Base node cannot have any children, skipping call");
	return nullptr;
}

void SableUI::ContentPanel::Update(const DrawableDrawData& drData)
{
	if (m_component == nullptr)
	{
		m_component = SB_new<BaseComponent>();
		m_component->SetRenderer(m_renderer);
		m_component->BackendInitialisePanel();
	}

	SableUI::Rect realRect = rect;
	
	auto* splitter = dynamic_cast<SplitterPanel*>(parent);
	if (splitter != nullptr)
	{
		realRect.x;
		realRect.y;
		realRect.w;
		realRect.h;
	}

	m_component->GetRootElement()->SetRect(drData.cmd, realRect);
	m_component->GetRootElement()->LayoutChildren(drData.cmd);
	Render(drData);
}

void SableUI::ContentPanel::Render(const DrawableDrawData& drData)
{
	if (m_component)
		m_component->Render(drData);
}

void SableUI::ContentPanel::DistributeEvents(const UIInputState& ctx, int z)
{
	if (m_component)
		m_component->HandleInput(ctx, z);
}

bool SableUI::ContentPanel::UpdateComponents(const DrawableDrawData& drData)
{
	if (!m_component)
		return false;

	bool changed = m_component->CheckAndUpdate(drData);

	if (changed)
		Update(drData);

	return changed;
}

void SableUI::ContentPanel::PostLayoutUpdate(const UIInputState& ctx, int z)
{
	if (m_component)
		m_component->PostLayoutUpdate(ctx, z);
}


SableUI::Element* SableUI::ContentPanel::GetElementById(const SableString& id)
{
	if (!m_component)
		return nullptr;

	return m_component->GetElementById(id);
}
