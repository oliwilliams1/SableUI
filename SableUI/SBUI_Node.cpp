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

	root->xPx  = 0;
	root->yPx  = 0;
	root->wPx  = wPx;
	root->hPx  = hPx;
	root->wFac = 1.0f;
	root->hFac = 1.0f;
}