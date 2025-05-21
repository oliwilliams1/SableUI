#include "SableUI/node.h"

SableUI::Node::Node(SableUI::NodeType type, SableUI::Node* parent, const std::string& name)
	: type(type), parent(parent), name(name)
{
	if (type != SableUI::NodeType::ROOTNODE)
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

void SableUI::SetupRootNode(Node* root, uint16_t wPx, uint16_t hPx)
{
	if (root->type != SableUI::NodeType::ROOTNODE)
	{
		SableUI_Error("Setting up a non-root node");
		return;
	}

	root->rect = { 0, 0, static_cast<float>(wPx), static_cast<float>(hPx)};
}