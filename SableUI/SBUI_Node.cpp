#include "SBUI_Node.h"

SableUI_node::SableUI_node(NodeType type, SableUI_node* parent, const std::string& name, Drawable::size s)
	: type(type), parent(parent), name(name)
{
	if (type != NodeType::ROOTNODE)
	{
		if (parent == nullptr)
		{
			printf("Parent node is null on a non-root node\n");
			return;
		}
		parent->children.push_back(this);
		index = static_cast<uint16_t>(parent->children.size() - 1);
	}
	else
	{
		index = 0;
	}

	if (s.v != -1.0f && size.type == Drawable::SizeType::PERCENT)
	{
		if (parent == nullptr) return;

		if (parent->type == NodeType::VSPLITTER)
		{
			parent->scaleFac.y = s.v / 100.0f;
		}

		if (parent->type == NodeType::HSPLITTER)
		{
			parent->scaleFac.x = s.v / 100.0f;
		}
	}
}

void SetupRootNode(SableUI_node* root, uint16_t wPx, uint16_t hPx)
{
	if (root->type != NodeType::ROOTNODE)
	{
		printf("Setting up a non-root node\n");
		return;
	}

	root->rect = { 0, 0, static_cast<float>(wPx), static_cast<float>(hPx)};
	root->scaleFac = { 1.0f, 1.0f };
}