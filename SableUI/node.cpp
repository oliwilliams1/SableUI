#include "SableUI/node.h"

SableUI_node::SableUI_node(NodeType type, SableUI_node* parent, const std::string& name)
	: type(type), parent(parent), name(name)
{
	if (type != NodeType::ROOTNODE)
	{
		if (parent == nullptr)
		{
			SableUI_Error("Parent node is null on a non-root node");
			return;
		}
		parent->children.push_back(this);
		index = static_cast<uint16_t>(parent->children.size() - 1);
	}
	else
	{
		index = 0;
	}
}

void SetupRootNode(SableUI_node* root, uint16_t wPx, uint16_t hPx)
{
	if (root->type != NodeType::ROOTNODE)
	{
		SableUI_Error("Setting up a non-root node");
		return;
	}

	root->rect = { 0, 0, static_cast<float>(wPx), static_cast<float>(hPx)};
}