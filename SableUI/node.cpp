#include "SableUI/node.h"

/* node */
SableUI::Node::Node(Node* parent, Renderer* renderer, const char* name) : parent(parent), m_renderer(renderer), name(name)
{
	SableUI_Log("Initalised %s", name);
};

/* root node */
SableUI::RootNode::RootNode(Renderer* renderer, int w, int h) : Node(nullptr, renderer, "Root Node")
{
	type = NodeType::ROOTNODE;
	rect.w = w;
	rect.h = h;
}

SableUI::RootNode::~RootNode()
{
	for (SableUI::Node* child : children)
	{
		delete child;
	}

	children.clear();
}

void SableUI::RootNode::Render()
{
	for (SableUI::Node* child : children)
	{
		child->Render();
	}
}

void SableUI::RootNode::Recalculate()
{
	for (SableUI::Node* child : children)
	{
		child->Recalculate();
	}
}

void SableUI::RootNode::DebugDrawBounds()
{
	for (SableUI::Node* child : children)
	{
		child->DebugDrawBounds();
	}
}

void SableUI::RootNode::AddChild(Node* node)
{
	if (children.size() > 0)
	{
		SableUI_Error("Root node can only have one child node");
		return;
	}

	children.push_back(node);
}

void SableUI::RootNode::Resize(int w, int h)
{
	rect.w = w;
	rect.h = h;
}

/* splitter node */
void SableUI::SplitterNode::Render()
{
	for (SableUI::Node* child : children)
	{
		child->Render();
	}
}

void SableUI::SplitterNode::Recalculate()
{
	for (SableUI::Node* child : children)
	{
		child->Recalculate();
	}
}

void SableUI::SplitterNode::DebugDrawBounds()
{
	for (SableUI::Node* child : children)
	{
		child->DebugDrawBounds();
	}
}

void SableUI::SplitterNode::AddChild(Node* node)
{
	children.push_back(node);
}

SableUI::SplitterNode::~SplitterNode()
{
	for (Node* child : children)
	{
		delete child;
	}

	children.clear();
}

/* base node */
SableUI::BaseNode::BaseNode(Node* parent, Renderer* renderer, const char* name) : Node(parent, renderer, name)
{
	type = NodeType::BASE;
}

void SableUI::BaseNode::DebugDrawBounds()
{
	rect.print();
	m_renderer->DirectDrawRect(rect, Colour(255, 0, 0));
}

void SableUI::BaseNode::AddChild(Node* node)
{
	SableUI_Error("Cannot add child to base node!");
}
