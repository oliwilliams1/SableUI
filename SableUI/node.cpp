#include "node.h"

Node::Node(NodeType type, Node* parent, const std::string& name, int id)
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

	if (parent == nullptr && type == NodeType::ROOTNODE) return; // Root Node

	switch (parent->type)
	{
	case NodeType::ROOTNODE:
		wFac = 1.0f;
		hFac = 1.0f;
		break;

	case NodeType::COMPONENT:
		printf("Cannot create a child node on a component node\n");
		return;

	case NodeType::HSPLITTER:
		wFac = 1.0f / parent->children.size();
		hFac = 1.0f;
		break;

	case NodeType::VSPLITTER:
		wFac = 1.0f;
		hFac = 1.0f / parent->children.size();
		break;

	default:
		printf("Unknown parent node type\n");
		return;
	}
}

void SetupRootNode(Node* root, uint16_t wPx, uint16_t hPx)
{
	if (root->type != NodeType::ROOTNODE)
	{
		printf("Setting up a non-root node\n");
		return;
	}

	root->wPx = wPx;
	root->hPx = hPx;
}