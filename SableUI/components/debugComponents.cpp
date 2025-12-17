#include <SableUI/components/debugComponents.h>
#include <SableUI/scrollContext.h>
#include <SableUI/component.h>
#include <SableUI/textCache.h>
#include <SableUI/renderer.h>
#include <SableUI/drawable.h>
#include <SableUI/texture.h>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <SableUI/string.h>
#include <SableUI/events.h>
#include <SableUI/window.h>
#include <SableUI/memory.h>
#include <SableUI/panel.h>
#include <SableUI/utils.h>
#include <SableUI/text.h>
#include <string>

using namespace SableUI;

static size_t g_selectedHash = 0;
static size_t g_hoveredHash = 0;
static Rect g_hoveredRect = {};
static ElementInfo g_hoveredInfo = {};
static Rect g_hoveredMinBounds = {};
static bool g_needsTransparencyUpdate = false;

// ============================================================================
// Helper functions
// ============================================================================
static SableString GetPanelName(BasePanel* panel)
{
	switch (panel->type)
	{
	case PanelType::BASE: return "content panel";
	case PanelType::HORIZONTAL: return "h-splitter";
	case PanelType::VERTICAL: return "v-splitter";
	case PanelType::Undef: return "undefined panel type";
	case PanelType::ROOTNODE: return "root";
	default: return "unknown panel type";
	}
}

static SableString ElementTypeToString(ElementType type)
{
	switch (type)
	{
	case ElementType::Undef: return "undefined element";
	case ElementType::RECT: return "rect";
	case ElementType::IMAGE: return "image";
	case ElementType::TEXT: return "text";
	case ElementType::DIV: return "div";
	default: return "unknown element type";
	}
}

static SableString LayoutDirectionToString(LayoutDirection dir)
{
	switch (dir)
	{
	case LayoutDirection::DOWN_UP: return "down-up";
	case LayoutDirection::UP_DOWN: return "up-down";
	case LayoutDirection::LEFT_RIGHT: return "left-right";
	case LayoutDirection::RIGHT_LEFT: return "right-left";
	default: return "";
	}
}

static SableString TextJustificationToString(TextJustification justify)
{
	switch (justify)
	{
	case TextJustification::Left: return "left";
	case TextJustification::Right: return "right";
	case TextJustification::Center: return "center";
	default: return "";
	}
}

// ============================================================================
// Memory Debugger
// ============================================================================
void SableUI::MemoryDebugger::Layout()
{
	Div(p(6) w_fill h_fill)
	{
		Text(SableString::Format("Click to toggle live update: %s", (live) ? "true" : "false"),
			onClick([this]() { setLive(!live); }));

		SplitterWithText("Panels");
		Text(SableString::Format("Base Panels: %d", BasePanel::GetNumInstances()));
		Text(SableString::Format("Root Panels: %d", RootPanel::GetNumInstances()));
		Text(SableString::Format("Splitter Panels: %d", SplitterPanel::GetNumInstances()));
		Text(SableString::Format("Content Panels: %d", ContentPanel::GetNumInstances()));

		SplitterWithText("Layout Elements");
		Text(SableString::Format("Components: %d", BaseComponent::GetNumInstances()));
		Text(SableString::Format("Elements: %d    (%zukb)",
			Element::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::Element).sizeInKB));
		Text(SableString::Format("Virtual Elements: %d    (%zukb)",
			VirtualNode::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::VirtualNode).sizeInKB));

		SplitterWithText("Drawables");
		Text(SableString::Format("Drawable Base: %d", DrawableBase::GetNumInstances()));
		Text(SableString::Format("Drawable Text: %d    (%zukb)",
			DrawableText::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::DrawableText).sizeInKB));
		Text(SableString::Format("Drawable Rect: %d    (%zukb)",
			DrawableRect::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::DrawableRect).sizeInKB));
		Text(SableString::Format("Drawable Splitter: %d    (%zukb)",
			DrawableSplitter::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::DrawableSplitter).sizeInKB));
		Text(SableString::Format("Drawable Image: %d    (%zukb)",
			DrawableImage::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::DrawableImage).sizeInKB));
		Text(SableString::Format("GPU Objects: %d    (%zukb)",
			GpuObject::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::GpuObject).sizeInKB));
		Text(SableString::Format("CustomDrawTargets: %d",
			CustomTargetQueue::GetNumInstances()));

		SplitterWithText("Utilities");
		Text(SableString::Format("Text: %d", _Text::GetNumInstances()));
		Text(SableString::Format("Textures: %d", Texture::GetNumInstances()));
		Text(SableString::Format("Strings: %d", String::GetNumInstances()));

		SplitterWithText("Font Manager");
		Text(SableString::Format("Font Packs: %d", FontPack::GetNumInstances()));
		Text(SableString::Format("Font Ranges: %d", FontRange::GetNumInstances()));

		int instanceCount = 0;
		for (const TextCacheFactory* factory : TextCacheFactory::GetFactories())
		{
			instanceCount++;
			Text(SableString::Format("Instance %d Text Cache: %d",
				instanceCount, factory->GetNumInstances()));
		}
	}
}

void SableUI::MemoryDebugger::OnUpdate(const UIEventContext& ctx)
{
	if (live) needsRerender = true;
}

// ============================================================================
// Layout Debugger
// ============================================================================
SableUI::LayoutDebugger::LayoutDebugger() : BaseComponent()
{
	tabs.Add("Layout Debugger").Add("Memory Diagnostics");
}

void SableUI::LayoutDebugger::Layout()
{
	SableUI::RenderTabHeader(tabs);

	Div(w_fill h_fill)
	{
		switch (tabs.activeTab)
		{
		case 0:
		{
			Div(p(6) w_fill h_fill)
			{
				SableString toggleHighlightElements = highlightElements ? "off" : "on";
				Text("Turn " + toggleHighlightElements + " highlight selected elements",
					mb(4)
					onClick([this]() {
						setHighlightElements(!highlightElements);
						}));

				SableString transparencyValueStr = SableString::Format("%d",
					((transparency * 100 / 255 + 10) / 20) * 20);
				Text("Set transparency " + transparencyValueStr + "%",
					mb(4)
					onClick([this]() {
						setTransparency(transparency + 52);
						if (transparency >= 255)
							setTransparency(0);
						g_needsTransparencyUpdate = true;
						}));

				ScrollViewCtx(treeScroll, bg(24, 24, 24))
				{
					for (int i = 0; i < 50; i++)
					{
						Div(w(50) h(50) m(4) bg(255, 0, 0)
							onDoubleClick([i]() { SableUI_Log("Clicked %d", i); }))
						{
							Rect(w(25) h(25) bg(0, 255, 0));
						}
					}
				}
			}
			break;
		}
		case 1:
		{
			Component("MemoryDebugger", w_fill h_fill bg(32, 32, 32));
			break;
		}
		default:
			Text("Underfined tab content", w_fill h_fill);
		}
	}
}

void SableUI::LayoutDebugger::OnUpdate(const UIEventContext& ctx)
{
	TabUpdateHandler(tabs);
	ScrollUpdateHandler(treeScroll);
}

// ============================================================================
// Element Tree View
// ============================================================================
void SableUI::ElementTreeView::InitData(Window* window, bool highlightElements, int transparency)
{
	this->window = window;
	this->highlightElements = highlightElements;
	this->transparency = transparency;
}

size_t SableUI::ElementTreeView::ComputeHash(const void* ptr, size_t parentHash) const
{
	size_t h = reinterpret_cast<size_t>(ptr);
	h ^= parentHash + 0x9e3779b9 + (h << 6) + (h >> 2);
	return h;
}

void SableUI::ElementTreeView::DrawElementTree(Element* element, int depth, int& line, size_t parentHash)
{
	if (!element) return;

	line++;
	size_t nodeHash = ComputeHash(element, parentHash);
	bool isExpanded = expandedNodes.count(nodeHash) > 0;
	bool hasChildren = !element->children.empty();

	SableString indent = std::string(2 * depth, ' ');
	SableString prefix = hasChildren ? (isExpanded ? U"\u25BE " : U"\u25B8 ") : U"    ";
	SableString name = element->ID.empty() ? ElementTypeToString(element->type) : element->ID;

	Div(h_fit w_fill bg(line % 2 == 0 ? rgb(40, 40, 40) : rgb(43, 43, 43)))
	{
		Text(indent + prefix + name,
			onDoubleClick([this, nodeHash, isExpanded]() {
				auto newExpanded = expandedNodes;
				if (isExpanded)
					newExpanded.erase(nodeHash);
				else
					newExpanded.insert(nodeHash);
				setExpandedNodes(newExpanded);
			})
			onClick([nodeHash, element]() {
				g_selectedHash = nodeHash;
			})
			onHover([this, nodeHash, element]() {
				if (g_hoveredHash != nodeHash) {
					g_hoveredHash = nodeHash;
					g_hoveredRect = element->rect;
					g_hoveredInfo = element->GetInfo();
					if (highlightElements) {
						g_hoveredMinBounds = {
							element->rect.x, element->rect.y,
							element->GetMinWidth(), element->GetMinHeight()
						};
					}
				}
			})
		);
	}

	if (isExpanded)
	{
		for (Child* child : element->children)
		{
			Element* el = (Element*)*child;
			DrawElementTree(el, depth + 1, line, nodeHash);
		}
	}
}

void SableUI::ElementTreeView::DrawPanelTree(BasePanel* panel, int depth, int& line, size_t parentHash)
{
	if (!panel) return;

	line++;
	size_t nodeHash = ComputeHash(panel, parentHash);
	bool isExpanded = expandedNodes.count(nodeHash) > 0;
	bool hasChildren = !panel->children.empty();

	SableString indent = std::string(2 * depth, ' ');
	SableString prefix = hasChildren ? (isExpanded ? U"\u25BE " : U"\u25B8 ") : U"    ";
	SableString name = GetPanelName(panel);

	Div(h_fit w_fill bg(line % 2 == 0 ? rgb(40, 40, 40) : rgb(43, 43, 43)))
	{
		Text(indent + prefix + name,
			onDoubleClick([this, nodeHash, isExpanded]() {
				auto newExpanded = expandedNodes;
				if (isExpanded)
					newExpanded.erase(nodeHash);
				else
					newExpanded.insert(nodeHash);
				setExpandedNodes(newExpanded);
			})
			onClick([nodeHash, panel]() {
				g_selectedHash = nodeHash;
			})
			onHover([this, nodeHash, panel]() {
				if (g_hoveredHash != nodeHash) {
					g_hoveredHash = nodeHash;
					g_hoveredRect = panel->rect;
					g_hoveredInfo = ElementInfo{};
					if (highlightElements) {
						g_hoveredMinBounds = {
							panel->rect.x, panel->rect.y,
							panel->minBounds.x, panel->minBounds.y
						};
					}
				}
			})
		);
	}

	if (isExpanded)
	{
		for (BasePanel* child : panel->children)
		{
			DrawPanelTree(child, depth + 1, line, nodeHash);
		}

		if (panel->children.empty())
		{
			if (ContentPanel* p = dynamic_cast<ContentPanel*>(panel))
			{
				DrawElementTree(p->GetComponent()->GetRootElement(), depth + 1, line, nodeHash);
			}
		}
	}
}

void SableUI::ElementTreeView::Layout()
{
	if (window == nullptr)
	{
		Text("Window is not set",
			centerY textColour(255, 0, 0));
		return;
	}

	int line = 0;
	DrawPanelTree(window->GetRoot(), 0, line, 0);
}

void SableUI::ElementTreeView::OnUpdate(const UIEventContext& ctx)
{
	if (window == nullptr) return;

	if (highlightElements &&
		(g_hoveredHash != lastDrawnHoveredHash || g_needsTransparencyUpdate))
	{
		UseCustomLayoutContext(queue, window, window->GetSurface())
		{
			queue->AddRect(window->GetRoot()->rect, Colour(0, 0, 0, transparency));
			g_needsTransparencyUpdate = false;

			lastDrawnHoveredHash = g_hoveredHash;
			const Rect& rect = g_hoveredRect;
			const ElementInfo& info = g_hoveredInfo;

			// padding
			if (info.paddingTop > 0) {
				queue->AddRect({ rect.x + info.paddingLeft, rect.y,
					rect.w - info.paddingLeft - info.paddingRight, info.paddingTop },
					Colour(140, 200, 140, 120));
			}
			if (info.paddingBottom > 0) {
				queue->AddRect({ rect.x + info.paddingLeft,
					rect.y + rect.h - info.paddingBottom,
					rect.w - info.paddingLeft - info.paddingRight, info.paddingBottom },
					Colour(140, 200, 140, 120));
			}
			if (info.paddingLeft > 0) {
				queue->AddRect({ rect.x, rect.y, info.paddingLeft, rect.h },
					Colour(140, 200, 140, 120));
			}
			if (info.paddingRight > 0) {
				queue->AddRect({ rect.x + rect.w - info.paddingRight, rect.y,
					info.paddingRight, rect.h },
					Colour(140, 200, 140, 120));
			}

			// margin
			if (info.marginTop > 0) {
				queue->AddRect({ rect.x, rect.y - info.marginTop, rect.w, info.marginTop },
					Colour(255, 180, 100, 120));
			}
			if (info.marginBottom > 0) {
				queue->AddRect({ rect.x, rect.y + rect.h, rect.w, info.marginBottom },
					Colour(255, 180, 100, 120));
			}
			if (info.marginLeft > 0) {
				queue->AddRect({ rect.x - info.marginLeft, rect.y - info.marginTop,
					info.marginLeft, rect.h + info.marginTop + info.marginBottom },
					Colour(255, 180, 100, 120));
			}
			if (info.marginRight > 0) {
				queue->AddRect({ rect.x + rect.w, rect.y - info.marginTop,
					info.marginRight, rect.h + info.marginTop + info.marginBottom },
					Colour(255, 180, 100, 120));
			}

			// min bounds
			queue->AddRect(g_hoveredMinBounds, Colour(255, 180, 100, 120));

			// content area
			Rect contentRect = {
				rect.x + info.paddingLeft,
				rect.y + info.paddingTop,
				rect.w - info.paddingLeft - info.paddingRight,
				rect.h - info.paddingTop - info.paddingBottom
			};
			queue->AddRect(contentRect, Colour(100, 180, 255, 120));
		}
	}
	else
	{
		RemoveQueueFromContext(queue);
	}
}

// ============================================================================
// Properties Panel
// ============================================================================
void SableUI::PropertiesPanel::Layout()
{
	Text(SableString("Properties").bold(), w_fill fontSize(14) m(6) mb(0));

	SableString title = selectedInfo.id.empty() ? "Element" : selectedInfo.id;
	SplitterWithText(title);

	Div(bg(32, 32, 32) p(6) w_fill)
	{
		Text("Bounding Box", textColour(180, 180, 180) mb(2));
		Text(selectedRect.ToString(), mb(8));

		Text("Layout Direction", textColour(180, 180, 180) mb(2));
		Text(LayoutDirectionToString(selectedInfo.layoutDirection), mb(8));

		Text("Constraints", textColour(180, 180, 180) mb(2));
		Text(SableString::Format("{ min-h: %d, max-h: %d}",
			selectedInfo.minHeight, selectedInfo.maxHeight), mb(2));
		Text(SableString::Format("{ min-w: %d, max-w: %d}",
			selectedInfo.minWidth, selectedInfo.maxWidth), mb(8));

		Text("Margins", textColour(180, 180, 180) mb(2));
		Text(SableString::Format("{ top: %d, right: %d, bottom: %d, left: %d }",
			selectedInfo.marginTop, selectedInfo.marginRight,
			selectedInfo.marginBottom, selectedInfo.marginLeft), mb(8));

		Text("Padding", textColour(180, 180, 180) mb(2));
		Text(SableString::Format("{ top: %d, right: %d, bottom: %d, left: %d }",
			selectedInfo.paddingTop, selectedInfo.paddingRight,
			selectedInfo.paddingBottom, selectedInfo.paddingLeft), mb(8));

		Text("Text Properties", textColour(180, 180, 180) mb(2));
		Text("Font Size: " + std::to_string(selectedInfo._fontSize), mb(2));
		Text("Line Height: " + std::to_string(selectedInfo._lineHeight), mb(2));
		Text("Text justification: " + TextJustificationToString(selectedInfo.textJustification), mb(8));

		Text("Positioning", textColour(180, 180, 180) mb(2));
		Text(SableString::Format("Center X: %s", selectedInfo._centerX ? "true" : "false"), mb(2));
		Text(SableString::Format("Center Y: %s", selectedInfo._centerY ? "true" : "false"), mb(8));

		Text("Clip rect", textColour(180, 180, 180) mb(2));
		Text(selectedClipRect.ToString(), mb(8));

		Text("Background Colour", textColour(180, 180, 180) mb(2));
		Rect(w(20) h(20) bg(selectedInfo.bgColour));
	}
}

void SableUI::PropertiesPanel::OnUpdate(const UIEventContext& ctx)
{
	if (lastSelectedHash != g_selectedHash)
	{
		setLastSelectedHash(g_selectedHash);
		setSelectedRect(g_hoveredRect);
		selectedInfo = g_hoveredInfo;
		setSelectedClipRect({});
	}
}