#include "SableUI/node.h"
#include "SableUI/component.h"

SableUI::Node::Node(Node* parent, Renderer* renderer) : parent(parent), m_renderer(renderer)
{
    type = NodeType::BASE;
    rect.wType = RectType::FILL;
    rect.hType = RectType::FILL;
}

/* Root node implementation */
SableUI::RootNode::RootNode(Renderer* renderer, int w, int h) : Node(nullptr, renderer)
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
        child->rect.x = rect.x;
        child->rect.y = rect.y;
        child->rect.w = rect.w;
        child->rect.h = rect.h;

        child->CalculateScales();
        child->CalculatePositions();
        child->CalculateMinBounds();
    }
}

SableUI::SplitterNode* SableUI::RootNode::AddSplitter(NodeType type)
{
    if (children.size() > 0)
    {
        SableUI_Error("Root node cannot have more than one child, dismissing call");
        return nullptr;
    }
    SplitterNode* node = new SplitterNode(this, type, m_renderer);
    children.push_back(node);

    Recalculate();
    return node;
}

SableUI::BaseNode* SableUI::RootNode::AddBaseNode()
{
    if (children.size() > 0)
    {
        SableUI_Error("Root node cannot have more than one child, dismissing call");
        return nullptr;
    }
    BaseNode* node = new BaseNode(this, m_renderer);
    children.push_back(node);

    Recalculate();
    return node;
}

SableUI::Node* SableUI::Node::FindRoot()
{
    Node* node = this;
    while (node->parent != nullptr) node = node->parent;
    return node;
}

void SableUI::RootNode::CalculateScales()
{
    SableUI_Error("Method does not exist for root node, use RootNode.Recalculate() instead");
    return;
}

void SableUI::RootNode::CalculatePositions()
{
    SableUI_Error("Method does not exist for root node, use RootNode.Recalculate() instead");
    return;
}

void SableUI::RootNode::Resize(int w, int h)
{
    rect.w = w;
    rect.h = h;
}

/* Splitter node implementation */
SableUI::SplitterNode::SplitterNode(Node* parent, NodeType type, Renderer* renderer) : Node(parent, renderer)
{
    this->type = type;
}

void SableUI::SplitterNode::Render()
{
    if (!m_drawableUpToDate) Update();
    for (SableUI::Node* child : children)
    {
        child->Render();
    }

    m_drawable.m_zIndex = 999;
    m_renderer->Draw(&m_drawable);
}

SableUI::SplitterNode* SableUI::SplitterNode::AddSplitter(NodeType type)
{
    SplitterNode* node = new SplitterNode(this, type, m_renderer);
    children.push_back(node);

    FindRoot()->Recalculate();
    return node;
}

SableUI::BaseNode* SableUI::SplitterNode::AddBaseNode()
{
    BaseNode* node = new BaseNode(this, m_renderer);
    children.push_back(node);

    FindRoot()->Recalculate();
    return node;
}

void SableUI::SplitterNode::CalculateScales()
{
    if (children.empty()) return;

    m_drawableUpToDate = false;

    if (type == NodeType::HSPLITTER)
    {
        int totalFixedWidth = 0;
        int numFillChildren = 0;
        std::vector<Node*> fillChildren;

        for (Node* child : children)
        {
            child->rect.h = rect.h;
            child->rect.hType = RectType::FILL;

            if (child->rect.wType == RectType::FIXED || child->rect.wType == RectType::FIT_CONTENT)
            {
                child->rect.w = (child->rect.wType == RectType::FIXED) ? child->rect.w : std::max(0, child->minBounds.x);
                totalFixedWidth += child->rect.w;
            }
            else
            {
                fillChildren.push_back(child);
                numFillChildren++;
            }
        }

        int availableWidth = rect.w - totalFixedWidth;

        if (numFillChildren > 0)
        {
            int widthPerFillChild = availableWidth / numFillChildren;
            int leftoverWidth = availableWidth % numFillChildren;

            for (size_t i = 0; i < fillChildren.size(); i++)
            {
                fillChildren[i]->rect.w = widthPerFillChild + (i < leftoverWidth ? 1 : 0);
            }
        }
    }
    else if (type == NodeType::VSPLITTER)
    {
        int totalFixedHeight = 0;
        int numFillChildren = 0;
        std::vector<Node*> fillChildren;

        for (Node* child : children)
        {
            child->rect.w = rect.w;
            child->rect.wType = RectType::FILL;

            if (child->rect.hType == RectType::FIXED || child->rect.hType == RectType::FIT_CONTENT)
            {
                child->rect.h = (child->rect.hType == RectType::FIXED) ? child->rect.h : std::max(0, child->minBounds.y);
                totalFixedHeight += child->rect.h;
            }
            else
            {
                fillChildren.push_back(child);
                numFillChildren++;
            }
        }

        int availableHeight = rect.h - totalFixedHeight;

        if (numFillChildren > 0)
        {
            int heightPerFillChild = availableHeight / numFillChildren;
            int leftoverHeight = availableHeight % numFillChildren;

            for (size_t i = 0; i < fillChildren.size(); i++)
            {
                fillChildren[i]->rect.h = heightPerFillChild + (i < leftoverHeight ? 1 : 0);
            }
        }
    }

    for (Node* child : children)
    {
        child->CalculateScales();
    }
}


void SableUI::SplitterNode::CalculatePositions()
{
    if (children.empty()) return;

    m_drawableUpToDate = false;
    bool toUpdate = false;

    vec2 cursor = { rect.x, rect.y };

    if (type == NodeType::HSPLITTER)
    {
        for (Node* child : children)
        {
            vec2 originalPosition = { child->rect.x, child->rect.y };

            child->rect.x = cursor.x;
            child->rect.y = cursor.y;

            if (child->rect.x != originalPosition.x || child->rect.y != originalPosition.y)
            {
                toUpdate = true;
            }

            cursor.x += child->rect.w;
        }
    }
    else if (type == NodeType::VSPLITTER)
    {
        for (Node* child : children)
        {
            vec2 originalPosition = { child->rect.x, child->rect.y };

            child->rect.x = cursor.x;
            child->rect.y = cursor.y;

            if (child->rect.x != originalPosition.x || child->rect.y != originalPosition.y)
            {
                toUpdate = true;
            }

            cursor.y += child->rect.h;
        }
    }

    for (Node* child : children)
    {
        bool childPositionChanged = false;
        child->CalculatePositions();
        if (childPositionChanged)
        {
            toUpdate = true;
        }
    }

    if (toUpdate) Update();
}

void SableUI::SplitterNode::CalculateMinBounds()
{
    if (children.empty())
    {
        minBounds = { 20, 20 };
        return;
    }

    if (type == NodeType::HSPLITTER)
    {
        int totalWidth = 0;
        int maxHeight = 0;

        for (Node* child : children)
        {
            child->CalculateMinBounds();
            totalWidth += child->rect.wType == SableUI::FILL ? child->minBounds.x : child->rect.w;
            maxHeight = std::max(maxHeight, child->rect.hType == SableUI::FILL ? child->minBounds.y : child->rect.h);
        }

        minBounds = { totalWidth, maxHeight };
    }
    else if (type == NodeType::VSPLITTER)
    {
        int totalHeight = 0;
        int maxWidth = 0;

        for (Node* child : children)
        {
            child->CalculateMinBounds();
            totalHeight += child->rect.hType == SableUI::FILL ? child->minBounds.y : child->rect.h;
            maxWidth = std::max(maxWidth, child->rect.wType == SableUI::FILL ? child->minBounds.x : child->rect.w);
        }

        minBounds = { maxWidth, totalHeight };
    }
}

void SableUI::SplitterNode::Update()
{
    std::vector<int> segments;

    for (SableUI::Node* child : children)
    {
        child->Update();

        if (type == NodeType::HSPLITTER)
        {
            segments.push_back(child->rect.x - rect.x);
        }
        if (type == NodeType::VSPLITTER)
		{
			segments.push_back(child->rect.y - rect.y);
		}
	}
    
    m_drawable.Update(rect, m_bColour, type, bSize, segments);
    m_drawableUpToDate = true;
    Render();
}

SableUI::SplitterNode::~SplitterNode()
{
    for (Node* child : children)
    {
        delete child;
    }
    children.clear();
}

/* Base Node Implementation */
static int s_ctr = 0;
SableUI::BaseNode::BaseNode(Node* parent, Renderer* renderer) : Node(parent, renderer)
{
    type = NodeType::BASE;

    /* Fixed, will update via code, should NOT be modified by algo */
    if (m_component == nullptr)
        m_component = new BaseComponent(renderer);
    Update();
}

SableUI::SplitterNode* SableUI::BaseNode::AddSplitter(NodeType type)
{
    SableUI_Error("Base node cannot have any children, skipping call");
    return nullptr;
}

SableUI::BaseNode* SableUI::BaseNode::AddBaseNode()
{
    SableUI_Error("Base node cannot have any children, skipping call");
    return nullptr;
}

void SableUI::BaseNode::Update()
{
    SableUI::Rect realRect = rect;
    
    if (auto* splitter = dynamic_cast<SplitterNode*>(parent); splitter != nullptr)
    {
        realRect.x += splitter->bSize;
		realRect.y += splitter->bSize;
        realRect.w -= splitter->bSize * 2;
        realRect.h -= splitter->bSize * 2;
    }

    m_component->GetBaseElement()->SetRect(realRect);
}

void SableUI::BaseNode::Render()
{
    m_component->GetBaseElement()->Render();
}