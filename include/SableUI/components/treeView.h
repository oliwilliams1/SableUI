#pragma once
#include <utility>

#include "SableUI/SableUI.h"

namespace SableUI
{
	struct TreeNode
	{
		SableString label;
		std::vector<TreeNode> children;
		bool isExpanded = false;

		auto operator<=>(const TreeNode&) const = default;
	};

	class TreeView final : public SableUI::BaseComponent
	{
	public:
		explicit TreeView(TreeNode  rootNode) : BaseComponent(), treeData(std::move(rootNode)) {}

		void SetData(const TreeNode& rootNode)
		{
			treeData = rootNode;
			needsRerender = true;
		}

		void Layout() override
		{
			if (!rootElement)
				return;

			nRow = 0;
			rootElement->setWType(SableUI::RectType::FILL);
			RenderNode(treeData, 0);
		}

	private:
		TreeNode treeData;
		int nRow = 0;

		useState(selectedNode, setSelectedNode, TreeNode*, nullptr);

		void RenderNode(TreeNode& node, int depth)
		{
			SableUI::Colour bgColour = (nRow % 2 == 0) ? rgb(43, 43, 43) : rgb(40, 40, 40);

			if (&node == selectedNode)
				bgColour = SableUI::Colour(bgColour.r + 40, bgColour.g + 40, bgColour.b + 40);

			Div(p(3) h_fit w_fill bg(bgColour)
				onClick([&]() {
					node.isExpanded = !node.isExpanded;
					setSelectedNode(&node);
					needsRerender = true;
				}))
			{
				SableString indicator;
				if (!node.children.empty())
					indicator = node.isExpanded ? U"▾ " : U"▸ ";
				else
					indicator = U"⦁ ";

				TextU32(indicator + node.label, ml(depth * 8));
			}

			nRow++;

			if (node.isExpanded && !node.children.empty())
			{
				for (auto& child : node.children)
					RenderNode(child, depth + 1);
			}
		}
	};
}