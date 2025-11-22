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
		ElementInfo elInfo;
		size_t uuid = 0;

		friend bool operator==(const TreeNode& a, const TreeNode& b)
		{
			if (a.name != b.name) return false;
			if (a.uuid != b.uuid) return false;
			bool res = a.children.size() == b.children.size();
			if (!res) return false;
			for (int i = 0; i < a.children.size(); i++)
				if (a.children[i] != b.children[i]) return false;
			return true;
		}
		friend bool operator!=(const TreeNode& a, const TreeNode& b)
			{ return !(a == b); }
	};

	class ElementTreeView : public SableUI::BaseComponent
	{
	public:
		ElementTreeView(SableUI::Window* window);
		TreeNode GenerateElementTree(Element* element, size_t& uuidCounter);
		TreeNode GeneratePanelTree(BasePanel* panel, size_t& uuidCounter);
		int DrawTreeNode(TreeNode& node, int depth, int line = 0);
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
		void PreserveExpandedState(const TreeNode& oldNode, TreeNode& newNode);
		void FindAndToggleNode(size_t node);

	private:
		useState(memoryDebugger, setMemoryDebugger, bool, false);
		useState(rootNode, setRootNode, TreeNode, {});
		Window* m_window = nullptr;
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