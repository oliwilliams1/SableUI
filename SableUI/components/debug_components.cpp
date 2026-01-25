#include <SableUI/components/debug_components.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/core/panel.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/scroll_context.h>
#include <SableUI/core/tab_context.h>
#include <SableUI/core/text.h>
#include <SableUI/core/texture.h>
#include <SableUI/styles/styles.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/memory.h>
#include <SableUI/utils/string.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/text_cache.h>
#include <string>

using namespace SableUI;
using namespace SableUI::Style;

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
	case PanelType::Base: return "content panel";
	case PanelType::HorizontalSplitter: return "h-splitter";
	case PanelType::VerticalSplitter: return "v-splitter";
	case PanelType::Undef: return "undefined panel type";
	case PanelType::Root: return "root";
	default: return "unknown panel type";
	}
}

static SableString ElementTypeToString(ElementType type)
{
	switch (type)
	{
	case ElementType::Undef: return "undefined element";
	case ElementType::Rect: return "rect";
	case ElementType::Image: return "image";
	case ElementType::Text: return "text";
	case ElementType::Div: return "div";
	default: return "unknown element type";
	}
}

static SableString LayoutDirectionToString(LayoutDirection dir)
{
	switch (dir)
	{
	case LayoutDirection::DownUp: return "down-up";
	case LayoutDirection::UpDown: return "up-down";
	case LayoutDirection::LeftRight: return "left-right";
	case LayoutDirection::RightLeft: return "right-left";
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
	Div(p(6), w_fill, h_fill)
	{
		TextSeperator("Panels");
		Text(SableString::Format("Base Panels: %d", BasePanel::GetNumInstances()));
		Text(SableString::Format("Root Panels: %d", RootPanel::GetNumInstances()));
		Text(SableString::Format("Splitter Panels: %d", SplitterPanel::GetNumInstances()));
		Text(SableString::Format("Content Panels: %d", ContentPanel::GetNumInstances()));

		TextSeperator("Layout Elements");
		Text(SableString::Format("Components: %d", BaseComponent::GetNumInstances()));
		Text(SableString::Format("Elements: %d    (%zukb)",
			Element::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::Element).sizeInKB));
		Text(SableString::Format("Virtual Elements: %d    (%zukb)",
			VirtualNode::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::VirtualNode).sizeInKB));

		TextSeperator("Drawables");
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

		TextSeperator("Utilities");
		Text(SableString::Format("Text: %d", _Text::GetNumInstances()));
		Text(SableString::Format("Textures: %d", Texture::GetNumInstances()));
		Text(SableString::Format("Strings: %d", String::GetNumInstances()));

		TextSeperator("Font Manager");
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
	if (live) MarkDirty();
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

	Div(w_fill, h_fill)
	{
		switch (tabs.activeTab)
		{
		case 0:
		{
			Div(p(6), w_fill, h_fill)
			{
				//Checkbox("Highlight Elements", highlightElements.get(), [this](bool checked) { highlightElements.set(checked); });

				SableString transparencyValueStr = SableString::Format("%d",
					((transparency * 100 / 255 + 10) / 20) * 20);
				Text("Set transparency " + transparencyValueStr + "%",
					mb(4),
					onClick([this]() {
						transparency.set(transparency.get() + 52);
						if (transparency.get() >= 255)
							transparency.set(0);
						g_needsTransparencyUpdate = true;
					}));

				ScrollViewCtx(treeScroll, bg(24, 24, 24))
				{
					for (int i = 0; i < 50; i++)
					{
						Div(w(50), h(50), m(4), bg(255, 0, 0),
							onDoubleClick([i]() { SableUI_Log("Clicked %d", i); }))
						{
							RectElement(w(25), h(25), bg(0, 255, 0));
						}
					}
				}
			}
			break;
		}
		case 1:
		{
			Component("MemoryDebugger", w_fill, h_fill, bg(32, 32, 32));
			break;
		}
		default:
			Text("Underfined tab content", w_fill, h_fill);
		}
	}
}

void SableUI::LayoutDebugger::OnUpdate(const UIEventContext& ctx)
{
	TabUpdateHandler(tabs);
	ScrollUpdateHandler(treeScroll);
}

void SableUI::LayoutDebugger::OnUpdatePostLayout(const UIEventContext& ctx)
{
	ScrollUpdatePostLayoutHandler(treeScroll);
}

// ============================================================================
// Properties Panel
// ============================================================================
void SableUI::PropertiesPanel::Layout()
{
	Text(SableString("Properties").bold(), w_fill, fontSize(14), m(6), mb(0));

	const ElementInfo& info = selectedInfo.get();

	SableString title = info.id.empty() ? "Element" : info.id;
	TextSeperator(title);

	Div(bg(32, 32, 32), p(6), w_fill)
	{
		Text("Bounding Box", textColour(180, 180, 180), mb(2));
		Text(selectedRect.get().ToString(), mb(8));

		Text("Layout Direction", textColour(180, 180, 180), mb(2));
		Text(LayoutDirectionToString(info.layout.layoutDirection), mb(8));

		Text("Constraints", textColour(180, 180, 180), mb(2));
		Text(SableString::Format("{ min-h: %d, max-h: %d }",
			info.layout.minH, info.layout.maxH), mb(2));
		Text(SableString::Format("{ min-w: %d, max-w: %d }",
			info.layout.minW, info.layout.maxW), mb(8));

		Text("Margins", textColour(180, 180, 180), mb(2));
		Text(SableString::Format("{ top: %d, right: %d, bottom: %d, left: %d }",
			info.layout.mT, info.layout.mR,
			info.layout.mB, info.layout.mL), mb(8));

		Text("Padding", textColour(180, 180, 180), mb(2));
		Text(SableString::Format("{ top: %d, right: %d, bottom: %d, left: %d }",
			info.layout.pT, info.layout.pR,
			info.layout.pB, info.layout.pL), mb(8));

		Text("Text Properties", textColour(180, 180, 180), mb(2));
		Text("Font Size: " + std::to_string(info.text.fontSize), mb(2));
		Text("Line Height: " + std::to_string(info.text.lineHeight), mb(2));
		Text("Text justification: " + TextJustificationToString(info.text.justification.value_or(TextJustification::Left)), mb(8));

		Text("Positioning", textColour(180, 180, 180), mb(2));
		Text(SableString::Format("Center X: %s", info.layout.centerX ? "true" : "false"), mb(2));
		Text(SableString::Format("Center Y: %s", info.layout.centerY ? "true" : "false"), mb(8));

		Text("Clip rect", textColour(180, 180, 180), mb(2));
		Text(selectedClipRect.get().ToString(), mb(8));

		Text("Background Colour", textColour(180, 180, 180), mb(2));
		RectElement(w(20), h(20), bg(info.appearance.bg.value_or(Colour{ 0, 0, 0, 0 })));
	}
}

void SableUI::PropertiesPanel::OnUpdate(const UIEventContext& ctx)
{
	if (lastSelectedHash != g_selectedHash)
	{
		lastSelectedHash.set(g_selectedHash);
		selectedRect.set(g_hoveredRect);
		selectedInfo.set(g_hoveredInfo);
		selectedClipRect.set({});
	}
}