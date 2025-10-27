#pragma once
#include "SableUI/SableUI.h"
#include "SableUI/element.h"

namespace SableUI
{
	SableString GetPanelName(BasePanel* panel)
	{
		switch (panel->type)
		{
		case SableUI::PanelType::BASE: return "content panel";
		case SableUI::PanelType::HORIZONTAL: return "h-splitter";
		case SableUI::PanelType::VERTICAL: return "v-splitter";
		case SableUI::PanelType::UNDEF: return "undefined panel type";
		case SableUI::PanelType::ROOTNODE: return "root";
		default: return "unknown panel type";
		}
	}

	SableString ElementTypeToString(ElementType type)
	{
		switch (type)
		{
		case SableUI::ElementType::UNDEF: return "undefined element";
		case SableUI::ElementType::RECT: return "rect";
		case SableUI::ElementType::IMAGE: return "image";
		case SableUI::ElementType::TEXT: return "text";
		case SableUI::ElementType::DIV: return "div";
		case SableUI::ElementType::TEXT_U32: return "text u32";
		default: return "unknown element type";
		}
	}

	struct TreeNode
	{
		SableString name;
		std::vector<TreeNode> children;
		bool isExpanded = true;

		friend bool operator==(const TreeNode& a, const TreeNode& b)
		{
			if (a.name != b.name) return false;
			bool res = a.children.size() == b.children.size();
			if (!res) return false;
			for (int i = 0; i < a.children.size(); i++)
				if (a.children[i] != b.children[i]) return false;

			return true;
		}

		friend bool operator!=(const TreeNode& a, const TreeNode& b)
		{
			return !(a == b);
		}
	};

	class DebugWindowView : public SableUI::BaseComponent
	{
	public:
		DebugWindowView(SableUI::Window* window) : SableUI::BaseComponent()
		{
			m_window = window;
		}

		TreeNode GenerateElementTree(Element* element)
		{
			TreeNode node;
			node.name = ElementTypeToString(element->type);

			for (Child* child : element->children)
			{
				Element* el = (Element*)*child;
				node.children.push_back(GenerateElementTree(el));
			}
			return node;
		}

		TreeNode GeneratePanelTree(BasePanel* panel)
		{
			TreeNode node;
			node.name = GetPanelName(panel);

			for (BasePanel* child : panel->children)
				node.children.push_back(GeneratePanelTree(child));

			if (panel->children.empty())
				if (ContentPanel* p = dynamic_cast<ContentPanel*>(panel))
					node.children.push_back(GenerateElementTree(p->GetComponent()->GetRootElement()));

			return node;
		}

		void DrawTreeNode(TreeNode& node, int depth)
		{
			SableString indent = std::string(2 * depth, ' ');

			SableString prefix;
			if (!node.children.empty())
				prefix = node.isExpanded ? U"\u25BE " : U"\u25B8 ";
			else
				prefix = "    ";

			TextU32(indent + prefix + node.name, onClick([&]() {
				node.isExpanded = !node.isExpanded;
				needsRerender = true;
			}));

			if (node.isExpanded)
			{
				for (TreeNode& child : node.children)
					DrawTreeNode(child, depth + 1);
			}
		}

		void Layout() override
		{
			rootElement->setPadding(4);
			Text("Open/close mem diagnostics", onClick([&]() { setMemoryDebugger(!memoryDebugger); }));

			if (memoryDebugger)
			{
				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
				Text("Base Panels: " + std::to_string(BasePanel::GetNumInstances()));
				Text("Root Panels: " + std::to_string(RootPanel::GetNumInstances()));
				Text("Splitter Panels: " + std::to_string(SplitterPanel::GetNumInstances()));
				Text("Content Panels: " + std::to_string(ContentPanel::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
				Text("Components: " + std::to_string(BaseComponent::GetNumInstances()));
				Text("Elements: " + std::to_string(Element::GetNumInstances()));
				Text("Virtual Elements: " + std::to_string(VirtualNode::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
				Text("Drawable Base: " + std::to_string(DrawableBase::GetNumInstances()));
				Text("Drawable Text: " + std::to_string(DrawableText::GetNumInstances()));
				Text("Drawable Rect: " + std::to_string(DrawableRect::GetNumInstances()));
				Text("Drawable Splitter: " + std::to_string(DrawableSplitter::GetNumInstances()));
				Text("Drawable Image: " + std::to_string(DrawableImage::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
				Text("Text: " + std::to_string(_Text::GetNumInstances()));
				Text("Textures: " + std::to_string(Texture::GetNumInstances()));
				Text("Strings: " + std::to_string(String::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
				Text("Font Packs: " + std::to_string(FontPack::GetNumInstances()));
				Text("Font Ranges: " + std::to_string(FontRange::GetNumInstances()));
			}

			Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
			Text("Panel/Element Tree Snapshot:");
			DrawTreeNode(rootNode, 0);
		}

		void OnUpdate(const UIEventContext& ctx) override
		{
			TreeNode newRoot = GeneratePanelTree(m_window->GetRoot());

			PreserveExpandedState(rootNode, newRoot);

			if (newRoot != rootNode)
				setRootNode(std::move(newRoot));
		}

	private:
		void PreserveExpandedState(const TreeNode& oldNode, TreeNode& newNode)
		{
			if (oldNode.name == newNode.name)
				newNode.isExpanded = oldNode.isExpanded;

			for (size_t i = 0; i < newNode.children.size(); ++i)
				if (i < oldNode.children.size())
					PreserveExpandedState(oldNode.children[i], newNode.children[i]);
		}

		useState(memoryDebugger, setMemoryDebugger, bool, false);
		useState(rootNode, setRootNode, TreeNode, {});
		Window* m_window = nullptr;
	};
}