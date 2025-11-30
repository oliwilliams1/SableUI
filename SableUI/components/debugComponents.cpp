#include <SableUI/components/debugComponents.h>
#include <string>
#include <utility>
#include <SableUI/component.h>
#include <SableUI/drawable.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/panel.h>
#include <SableUI/renderer.h>
#include <SableUI/SableUI.h>
#include <SableUI/string.h>
#include <SableUI/text.h>
#include <SableUI/texture.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>
#include <SableUI/textCache.h>
#include <SableUI/memory.h>

using namespace SableUI;

static TreeNode g_selectedNode;
static TreeNode g_hoveredNode;
static TreeNode g_lastDrawnHoveredNode;
static size_t g_currentHoveredUUID = 0;
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
	case PanelType::UNDEF: return "undefined panel type";
	case PanelType::ROOTNODE: return "root";
	default: return "unknown panel type";
	}
}

static SableString ElementTypeToString(ElementType type)
{
	switch (type)
	{
	case ElementType::UNDEF: return "undefined element";
	case ElementType::RECT: return "rect";
	case ElementType::IMAGE: return "image";
	case ElementType::TEXT: return "text";
	case ElementType::DIV: return "div";
	case ElementType::TEXT_U32: return "text u32";
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
// Element Tree Viewer
// ============================================================================
SableUI::ElementTreeView::ElementTreeView(Window* window)
	: BaseComponent(), m_window(window) {};

TreeNode SableUI::ElementTreeView::GenerateElementTree(Element* element, size_t& uuidCounter)
{
	TreeNode node;
	node.name = ElementTypeToString(element->type);
	node.rect = element->rect;
	if (highlightElements)
		node.minBoundsRect = { element->rect.x, element->rect.y, element->GetMinWidth(), element->GetMinHeight() };
	
	node.elInfo = element->GetInfo();
	node.uuid = uuidCounter++;

	for (Child* child : element->children)
	{
		Element* el = (Element*)*child;
		node.children.push_back(GenerateElementTree(el, uuidCounter));
	}
	return node;
}

TreeNode SableUI::ElementTreeView::GeneratePanelTree(BasePanel* panel, size_t& uuidCounter)
{
	TreeNode node;
	node.name = GetPanelName(panel);
	node.rect = panel->rect;
	node.minBoundsRect = { panel->rect.x, panel->rect.y, panel->minBounds.x, panel->minBounds.y };
	node.uuid = uuidCounter++;

	for (BasePanel* child : panel->children)
		node.children.push_back(GeneratePanelTree(child, uuidCounter));

	if (panel->children.empty())
		if (ContentPanel* p = dynamic_cast<ContentPanel*>(panel))
			node.children.push_back(GenerateElementTree(p->GetComponent()->GetRootElement(), uuidCounter));

	return node;
}

int SableUI::ElementTreeView::DrawTreeNode(TreeNode& node, int depth, int line)
{
	line++;
	SableString indent = std::string(2 * depth, ' ');

	SableString prefix;
	if (!node.children.empty())
		prefix = node.isExpanded ? U"\u25BE " : U"\u25B8 ";
	else
		prefix = "    ";
	
	TreeNode nodeCopy = node;
	size_t nodeUUID = node.uuid;

	Div(h_fit w_fill bg(line % 2 == 0 ? rgb(40, 40, 40) : rgb(43, 43, 43)))
	{
		TextU32(indent + prefix + node.name,
			onDoubleClick([&, nodeUUID]() {
				FindAndToggleNode(nodeUUID);
			})
			onClick([nodeCopy]() {
				g_selectedNode = nodeCopy;
			})
			onHover([nodeCopy]() {
				g_currentHoveredUUID = nodeCopy.uuid;
				g_hoveredNode = nodeCopy;
			})
		);
	}

	if (node.isExpanded)
		for (TreeNode& child : node.children)
			line = DrawTreeNode(child, depth + 1, line);

	return line;
}

void SableUI::ElementTreeView::Layout()
{
	rootElement->setPadding(4);
	SableString memSwitch = memoryDebugger ? "Close" : "Open";
	Text(memSwitch + " memory diagnostics", onDoubleClick([&]() { setMemoryDebugger(!memoryDebugger); }));

	if (memoryDebugger)
	{
		Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
		Text(SableString::Format("Base Panels: %d", BasePanel::GetNumInstances()));
		Text(SableString::Format("Root Panels: %d", RootPanel::GetNumInstances()));
		Text(SableString::Format("Splitter Panels: %d", SplitterPanel::GetNumInstances()));
		Text(SableString::Format("Content Panels: %d", ContentPanel::GetNumInstances()));

		Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

		Text(SableString::Format("Components: %d", BaseComponent::GetNumInstances()));

		Text(SableString::Format("Elements: %d    (%zukb)", 
			Element::GetNumInstances(), 
			SableMemory::GetSizeData(SableMemory::PoolType::Element).sizeInKB));

		Text(SableString::Format("Virtual Elements: %d    (%zukb)",
			VirtualNode::GetNumInstances(),
			SableMemory::GetSizeData(SableMemory::PoolType::VirtualNode).sizeInKB));

		Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

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

		Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
		Text(SableString::Format("Text: %d", _Text::GetNumInstances()));
		Text(SableString::Format("Textures: %d", Texture::GetNumInstances()));
		Text(SableString::Format("Strings: %d", String::GetNumInstances()));

		Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
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

	Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

	SableString toggleHighlightElements = highlightElements ? "off" : "on";
	Text("Turn " + toggleHighlightElements + " highlight selected elements",
		mb(4)
		onClick([&]() { setHighlightElements(!highlightElements);
	}));

	SableString transparencyValueStr = SableString::Format("%d", ((transparency * 100 / 255 + 10) / 20) * 20);
	Text("Set transparency " + transparencyValueStr + "%",
		mb(4)
		onClick([&]() {
			setTransparency(transparency + 52);
			if (transparency >= 255)
				setTransparency(0);

			g_needsTransparencyUpdate = true;
		})
	);

	DrawTreeNode(rootNode, 0);
}

void SableUI::ElementTreeView::OnUpdate(const UIEventContext& ctx)
{
	size_t uuidCounter = 0;
	TreeNode newRoot = GeneratePanelTree(m_window->GetRoot(), uuidCounter);
	PreserveExpandedState(rootNode, newRoot);
	
	if (newRoot != rootNode)
		setRootNode(std::move(newRoot));
	
	if ((highlightElements && g_hoveredNode != g_lastDrawnHoveredNode) || g_needsTransparencyUpdate && highlightElements)
	{
		//CustomTargetQueue* queue = m_window->CreateCustomTargetQueue_window(m_window->GetWindowSurface());
		//queue->AddRect(rootNode.rect, Colour(0, 0, 0, transparency));
		//g_needsTransparencyUpdate = false;

		//g_lastDrawnHoveredNode = g_hoveredNode;
		//const Rect& rect = g_hoveredNode.rect;
		//const ElementInfo& info = g_hoveredNode.elInfo;

		//// Padding
		//if (info.paddingTop > 0) {
		//	Rect paddingTop = {
		//		rect.x + info.paddingLeft,
		//		rect.y,
		//		rect.w - info.paddingLeft - info.paddingRight,
		//		info.paddingTop
		//	};
		//	queue->AddRect(paddingTop, Colour(140, 200, 140, 120));
		//}
		//if (info.paddingBottom > 0) {
		//	Rect paddingBottom = {
		//		rect.x + info.paddingLeft,
		//		rect.y + rect.h - info.paddingBottom,
		//		rect.w - info.paddingLeft - info.paddingRight,
		//		info.paddingBottom
		//	};
		//	queue->AddRect(paddingBottom, Colour(140, 200, 140, 120));
		//}
		//if (info.paddingLeft > 0) {
		//	Rect paddingLeft = {
		//		rect.x,
		//		rect.y,
		//		info.paddingLeft,
		//		rect.h
		//	};
		//	queue->AddRect(paddingLeft, Colour(140, 200, 140, 120));
		//}
		//if (info.paddingRight > 0) {
		//	Rect paddingRight = {
		//		rect.x + rect.w - info.paddingRight,
		//		rect.y,
		//		info.paddingRight,
		//		rect.h
		//	};
		//	queue->AddRect(paddingRight, Colour(140, 200, 140, 120));
		//}

		//// Margins
		//if (info.marginTop > 0) {
		//	Rect marginTop = {
		//		rect.x,
		//		rect.y - info.marginTop,
		//		rect.w,
		//		info.marginTop
		//	};
		//	queue->AddRect(marginTop, Colour(255, 180, 100, 120));
		//}
		//if (info.marginBottom > 0) {
		//	Rect marginBottom = {
		//		rect.x,
		//		rect.y + rect.h,
		//		rect.w,
		//		info.marginBottom
		//	};
		//	queue->AddRect(marginBottom, Colour(255, 180, 100, 120));
		//}
		//if (info.marginLeft > 0) {
		//	Rect marginLeft = {
		//		rect.x - info.marginLeft,
		//		rect.y - info.marginTop,
		//		info.marginLeft,
		//		rect.h + info.marginTop + info.marginBottom
		//	};
		//	queue->AddRect(marginLeft, Colour(255, 180, 100, 120));
		//}
		//if (info.marginRight > 0) {
		//	Rect marginRight = {
		//		rect.x + rect.w,
		//		rect.y - info.marginTop,
		//		info.marginRight,
		//		rect.h + info.marginTop + info.marginBottom
		//	};
		//	queue->AddRect(marginRight, Colour(255, 180, 100, 120));
		//}

		//queue->AddRect(g_hoveredNode.minBoundsRect, Colour(255, 180, 100, 120));

		//// Content area
		//Rect contentRect = {
		//	rect.x + info.paddingLeft,
		//	rect.y + info.paddingTop,
		//	rect.w - info.paddingLeft - info.paddingRight,
		//	rect.h - info.paddingTop - info.paddingBottom
		//};
		//queue->AddRect(contentRect, Colour(100, 180, 255, 120));
	}
}

void SableUI::ElementTreeView::PreserveExpandedState(const TreeNode& oldNode, TreeNode& newNode)
{
	if (oldNode.name == newNode.name)
		newNode.isExpanded = oldNode.isExpanded;

	for (size_t i = 0; i < newNode.children.size(); ++i)
		if (i < oldNode.children.size())
			PreserveExpandedState(oldNode.children[i], newNode.children[i]);
}

void SableUI::ElementTreeView::FindAndToggleNode(size_t uuid)
{
	auto toggle = [](TreeNode& n, size_t id, auto& self) -> bool
	{
		if (n.uuid == id)
		{
			n.isExpanded = !n.isExpanded;
			return true;
		}
		for (auto& child : n.children)
			if (self(child, id, self)) return true;

		return false;
	};

	toggle(rootNode, uuid, toggle);
	needsRerender = true;
}

// ============================================================================
// Properties Viewer
// ============================================================================
void SableUI::PropertiesView::Layout()
{
	Div(left_right w_fill h_fit bg(32, 32, 32) p(6))
	{
		Text(SableString("Properties").bold(), w_fill fontSize(14));
		Text(selected.elInfo.id.size() > 0 ? selected.elInfo.id : "Element", 
			w_fill justify_right textColour(180, 180, 180));
	}

	Rect(mx(2) mt(4) h(1) w_fill bg(67, 67, 67));

	Div(bg(32, 32, 32) p(6) w_fill)
	{
		Text("Bounding Box", textColour(180, 180, 180) mb(2));
		Text(selected.rect.ToString(), mb(8));

		Text("Layout Direction", textColour(180, 180, 180) mb(2));
		Text(LayoutDirectionToString(selected.elInfo.layoutDirection), mb(8));

		Text("Contraints", textColour(180, 180, 180) mb(2));
		SableString wConstraints = SableString::Format("{ min-h: %d, max-h: %d}", selected.elInfo.minHeight, selected.elInfo.maxHeight);
		Text(wConstraints, mb(2));
		SableString hConstraints = SableString::Format("{ min-w: %d, max-w: %d}", selected.elInfo.minWidth, selected.elInfo.maxWidth);
		Text(hConstraints, mb(8));

		Text("Margins", textColour(180, 180, 180) mb(2));
		SableString margins = SableString::Format("{ top: %d, right: %d, bottom: %d, left: %d }",
			selected.elInfo.marginTop, selected.elInfo.marginRight, selected.elInfo.marginBottom, selected.elInfo.marginLeft);
		Text(margins, mb(8));

		Text("Padding", textColour(180, 180, 180) mb(2));
		SableString paddings = SableString::Format("{ top: %d, right: %d, bottom: %d, left: %d }",
			selected.elInfo.paddingTop, selected.elInfo.paddingRight, selected.elInfo.paddingBottom, selected.elInfo.paddingLeft);
		Text(paddings, mb(8));

		Text("Text Properties", textColour(180, 180, 180) mb(2));
		Text("Font Size: " + std::to_string(selected.elInfo._fontSize), mb(2));
		Text("Line Height: " + std::to_string(selected.elInfo._lineHeight), mb(2));
		Text("Text justification: " + TextJustificationToString(selected.elInfo.textJustification), mb(8));

		Text("Positioning", textColour(180, 180, 180) mb(2));
		Text(SableString::Format("Center X: %s", selected.elInfo._centerX ? "true" : "false"), mb(2));
		Text(SableString::Format("Center Y: %s", selected.elInfo._centerY ? "true" : "false"), mb(8));

		Text("Background Colour", textColour(180, 180, 180) mb(2));
		Rect(w(20) h(20) bg(selected.elInfo.bgColour));
	}
}

void SableUI::PropertiesView::OnUpdate(const UIEventContext& ctx)
{
	if (selected != g_selectedNode)
		setSelected(g_selectedNode);
}