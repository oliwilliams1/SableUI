#include "SBUI_Node.h"

SBUI_node::SBUI_node(NodeType type, SBUI_node* parent, const std::string& name, int id)
	: type(type), parent(parent), name(name), id(id)
{
	if (type != NodeType::ROOTNODE)
	{
		if (parent == nullptr)
		{
			printf("Parent node is null on a non-root node\n");
			return;
		}
		parent->children.push_back(this);
	}
}

void SetupRootNode(SBUI_node* root, uint16_t wPx, uint16_t hPx)
{
	if (root->type != NodeType::ROOTNODE)
	{
		printf("Setting up a non-root node\n");
		return;
	}

	root->wPx  = wPx;
	root->hPx  = hPx;
	root->wFac = 1.0f;
	root->hFac = 1.0f;
}