#include <algorithm>
#include <vector>
#include <SableUI/panel.h>
#include <SableUI/memory.h>
#include <SableUI/drawable.h>
#include <SableUI/component.h>
#include <SableUI/console.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/renderer.h>
#include <SableUI/utils.h>

using namespace SableMemory;

static int s_basePanelCount = 0;
SableUI::BasePanel::BasePanel(BasePanel* parent, RendererBackend* renderer) : parent(parent), m_renderer(renderer)
{
	type = PanelType::BASE;
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

void SableUI::BasePanel::PropagateEvents(const UIEventContext& ctx)
{
	for (BasePanel* child : children)
		child->PropagateEvents(ctx);
}

void SableUI::BasePanel::PropagatePostLayoutEvents(const UIEventContext& ctx)
{
	for (BasePanel* child : children)
		child->PropagatePostLayoutEvents(ctx);
}

bool SableUI::BasePanel::PropagateComponentStateChanges()
{
	bool res = false;
	for (BasePanel* child : children)
		res = res || child->PropagateComponentStateChanges();

	return res;
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
	type = PanelType::ROOTNODE;
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

void SableUI::RootPanel::Render()
{
	for (SableUI::BasePanel* child : children)
		child->Render();
}

void SableUI::RootPanel::Recalculate()
{
	for (SableUI::BasePanel* child : children)
	{
		child->rect.x = rect.x;
		child->rect.y = rect.y;
		child->rect.w = rect.w;
		child->rect.h = rect.h;

		child->CalculateScales();
		child->CalculatePositions();
		child->CalculateMinBounds();

		if (child->type == PanelType::BASE)
		{
			if (SableUI::ContentPanel* panelChild = dynamic_cast<SableUI::ContentPanel*>(child))
			{
				panelChild->Update();
			}
			else
			{
				SableUI_Error("Root child marked as BASE but not a Panel instance.");
			}
		}
	}
}

SableUI::SplitterPanel* SableUI::RootPanel::AddSplitter(PanelType type)
{
	if (children.size() > 0)
	{
		SableUI_Error("Root node cannot have more than one child, dismissing call");
		return nullptr;
	}
	SplitterPanel* node = SB_new<SplitterPanel>(this, type, m_renderer);
	children.push_back(node);

	Recalculate();
	return node;
}

SableUI::ContentPanel* SableUI::RootPanel::AddPanel()
{
	if (children.size() > 0)
	{
		SableUI_Error("Root node cannot have more than one child, dismissing call");
		return nullptr;
	}
	ContentPanel* node = SB_new<ContentPanel>(this, m_renderer);
	children.push_back(node);

	Recalculate();
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

void SableUI::RootPanel::CalculatePositions()
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

void SableUI::SplitterPanel::Render()
{
	if (!m_drawableUpToDate) Update();
	for (SableUI::BasePanel* child : children)
		child->Render();

	m_drawable->m_zIndex = 999;
	m_renderer->AddToDrawStack(m_drawable);
}

SableUI::SplitterPanel* SableUI::SplitterPanel::AddSplitter(PanelType type)
{
	SplitterPanel* node = SB_new<SplitterPanel>(this, type, m_renderer);
	children.push_back(node);

	FindRoot()->Recalculate();
	return node;
}

SableUI::ContentPanel* SableUI::SplitterPanel::AddPanel()
{
	ContentPanel* node = SB_new<ContentPanel>(this, m_renderer);
	children.push_back(node);

	FindRoot()->Recalculate();
	return node;
}

void SableUI::SplitterPanel::CalculateScales()
{
	if (children.empty()) return;

	m_drawableUpToDate = false;

	if (type == PanelType::HORIZONTAL)
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
	else if (type == PanelType::VERTICAL)
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

void SableUI::SplitterPanel::CalculatePositions()
{
	if (children.empty()) return;

	m_drawableUpToDate = false;
	bool toUpdate = false;

	vec2 cursor = { rect.x, rect.y };

	if (type == PanelType::HORIZONTAL)
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
	else if (type == PanelType::VERTICAL)
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
		child->CalculatePositions();
		if (childPositionChanged)
			toUpdate = true;
	}

	if (toUpdate) Update();
}

void SableUI::SplitterPanel::CalculateMinBounds()
{
	if (children.empty())
	{
		minBounds = { 20, 20 };
		return;
	}

	if (type == PanelType::HORIZONTAL)
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
	else if (type == PanelType::VERTICAL)
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

void SableUI::SplitterPanel::Update()
{
	std::vector<int> segments;

	for (SableUI::BasePanel* child : children)
	{
		child->Update();

		if (type == PanelType::HORIZONTAL)
			segments.push_back(child->rect.x - rect.x);
		if (type == PanelType::VERTICAL)
			segments.push_back(child->rect.y - rect.y);
	}
	
	m_drawable->Update(rect, m_bColour, type, bSize, segments);
	m_drawableUpToDate = true;
	Render();
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
// Root Panel
// ============================================================================
static int s_panelCount = 0; 
SableUI::ContentPanel::ContentPanel(BasePanel* parent, RendererBackend* renderer) : BasePanel(parent, renderer)
{
	s_panelCount++;
	type = PanelType::BASE;
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

SableUI::SplitterPanel* SableUI::ContentPanel::AddSplitter(PanelType type)
{
	SableUI_Error("Base node cannot have any children, skipping call");
	return nullptr;
}

SableUI::ContentPanel* SableUI::ContentPanel::AddPanel()
{
	SableUI_Error("Base node cannot have any children, skipping call");
	return nullptr;
}

void SableUI::ContentPanel::Update()
{
	if (m_component == nullptr)
	{
		m_component = SB_new<BaseComponent>();
		m_component->SetRenderer(m_renderer);
		m_component->BackendInitialisePanel();
	}

	SableUI::Rect realRect = rect;
	
	if (auto* splitter = dynamic_cast<SplitterPanel*>(parent); splitter != nullptr)
	{
		realRect.x += splitter->bSize;
		realRect.y += splitter->bSize;
		realRect.w -= splitter->bSize * 2;
		realRect.h -= splitter->bSize * 2;
	}

	m_component->GetRootElement()->SetRect(realRect);
	m_component->GetRootElement()->LayoutChildren();
	Render();
}

void SableUI::ContentPanel::Render()
{
	m_component->Render();
}

void SableUI::ContentPanel::PropagateEvents(const UIEventContext& ctx)
{
	m_component->comp_PropagateEvents(ctx);
}

void SableUI::ContentPanel::PropagatePostLayoutEvents(const UIEventContext& ctx)
{
	m_component->comp_PropagatePostLayoutEvents(ctx);
}

bool SableUI::ContentPanel::PropagateComponentStateChanges()
{
	bool res = false;
	if (m_component->comp_PropagateComponentStateChanges(&res))
	{
		m_component->Rerender(&res);
		m_component->GetRootElement()->LayoutChildren();
		Update();
	}

	return res;
}

SableUI::Element* SableUI::ContentPanel::GetElementById(const SableString& id)
{
	if (!m_component)
		return nullptr;

	return m_component->GetElementById(id);
}
