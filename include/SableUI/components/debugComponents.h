#pragma once
#include <vector>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <SableUI/component.h>
#include <SableUI/events.h>
#include <SableUI/panel.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>

namespace SableUI
{
	struct TreeNode
	{
		SableString name;
		std::vector<TreeNode> children;
		bool isExpanded = false;
		Rect rect;
		Rect minBoundsRect;
		ElementInfo elInfo;
		size_t uuid = 0;

		friend bool operator==(const TreeNode& a, const TreeNode& b)
		{
			if (a.name != b.name) return false;
			if (a.uuid != b.uuid) return false;
			if (a.children.size() != b.children.size())	return false;
			if (a.rect != b.rect) return false;
			if (a.minBoundsRect != b.minBoundsRect) return false;

			for (int i = 0; i < a.children.size(); i++)
				if (a.children[i] != b.children[i]) return false;
			return true;
		}
		friend bool operator!=(const TreeNode& a, const TreeNode& b)
			{ return !(a == b); }
	};

	class MemoryDebugger : public SableUI::BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

	private:
		useState(live, setLive, bool, true);
	};

	class ElementTreeView : public SableUI::BaseComponent
	{
	public:
		TreeNode GenerateElementTree(Element* element, size_t& uuidCounter);
		TreeNode GeneratePanelTree(BasePanel* panel, size_t& uuidCounter);
		int DrawTreeNode(TreeNode& node, int depth, int line = 0);
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
		void PreserveExpandedState(const TreeNode& oldNode, TreeNode& newNode);
		void FindAndToggleNode(size_t node);
		void SetWindow(Window* window) { setWindow(window); }

	private:
		useState(highlightElements, setHighlightElements, bool, false);
		useState(transparency, setTransparency, int, 0);
		useState(rootNode, setRootNode, TreeNode, {}); 
		useState(window, setWindow, Window*, nullptr);
		CustomLayoutContext(queue);
	};

	class PropertiesView : public SableUI::BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

	private:
		useState(selected, setSelected, TreeNode, {});
	};
}