#include "SableUI/node.h"

SableUI::Node::Node(SableUI::NodeType type, SableUI::Node* parent, const std::string& name, Renderer* renderer)
	: type(type), parent(parent), name(name), m_renderer(renderer)
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

SableUI::Node* SableUI::Node::AttachSplitter(SableUI::Colour colour, int borderSize)
{
	if (m_component != nullptr)
	{
		SableUI_Error("Node: %s already has a component attached!", name.c_str());
		return this;
	}

	bSize = borderSize;

	m_component = new Splitter(this, colour);
	
	m_component->SetParent(this);
	m_component->SetRenderer(m_renderer);

	return this;
}

SableUI::Node* SableUI::Node::AttachBase(SableUI::Colour colour)
{
	if (m_component != nullptr)
	{
		SableUI_Error("Node: %s already has a component attached!", name.c_str());
		return this;
	}

	m_component = new Body(this, colour);

	m_component->SetParent(this);
	m_component->SetRenderer(m_renderer);

	if (auto* base = dynamic_cast<Body*>(m_component))
	{
		ElementInfo info{};
		info.name = name + "_body";
		info.wType = RectType::FILL;
		info.hType = RectType::FILL;

		Element* element = m_renderer->CreateElement(info.name, ElementType::DIV);
		info.type = ElementType::DIV;
		element->SetInfo(info);

		base->AddElement(element);
		base->UpdateElements();
	}

	return this;
}

void SableUI::SetupRootNode(Node* root, uint16_t wPx, uint16_t hPx)
{
	if (root->type != SableUI::NodeType::ROOTNODE)
	{
		SableUI_Error("Setting up a non-root node");
		return;
	}

	root->rect = { 0, 0, static_cast<int>(wPx), static_cast<int>(hPx)};
}

SableUI::Node::~Node()
{
	delete m_component;
}