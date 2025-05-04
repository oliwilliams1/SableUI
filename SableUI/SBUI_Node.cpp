#include "SBUI_Node.h"

SbUI_node::SbUI_node(NodeType type, SbUI_node* parent, const std::string& name)
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
		index = parent->children.size() - 1;
	}
	else
	{
		index = 0;
	}
}

void SetupRootNode(SbUI_node* root, uint16_t wPx, uint16_t hPx)
{
	if (root->type != NodeType::ROOTNODE)
	{
		printf("Setting up a non-root node\n");
		return;
	}

	root->rect = { 0, 0, static_cast<float>(wPx), static_cast<float>(hPx )};
	root->scaleFac = { 1.0f, 1.0f };
}