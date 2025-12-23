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
	Div(p(6), w_fill, h_fill)
	{
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

	Div(w_fill, h_fill)
	{
		switch (tabs.activeTab)
		{
		case 0:
		{
			Div(p(6), w_fill, h_fill)
			{
				Checkbox("Highlight Elements", highlightElements.get(), [this](bool checked) { highlightElements.set(checked); });

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
							Rect(w(25), h(25), bg(0, 255, 0));
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
	SplitterWithText(title);

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
		Text("Text justification: " + TextJustificationToString(info.text.justification), mb(8));

		Text("Positioning", textColour(180, 180, 180), mb(2));
		Text(SableString::Format("Center X: %s", info.layout.centerX ? "true" : "false"), mb(2));
		Text(SableString::Format("Center Y: %s", info.layout.centerY ? "true" : "false"), mb(8));

		Text("Clip rect", textColour(180, 180, 180), mb(2));
		Text(selectedClipRect.get().ToString(), mb(8));

		Text("Background Colour", textColour(180, 180, 180), mb(2));
		Rect(w(20), h(20), bg(info.appearance.bg));
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