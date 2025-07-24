#include "SableUI/node.h"

SableUI::Node::Node(Node* parent, Renderer* renderer) : parent(parent), m_renderer(renderer)
{
    type = NodeType::BASE;
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

    for (Node* child : children)
    {
        child->CalculateScales();
    }

    if (type == NodeType::HSPLITTER)
    {
        int totalFixedWidth = 0;
        int totalFillMinWidth = 0;
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
                totalFillMinWidth += child->minBounds.x;
                numFillChildren++;
            }
        }

        int availableWidth = rect.w - totalFixedWidth;

        if (numFillChildren > 0)
        {
            if (availableWidth < totalFillMinWidth)
            {
                for (Node* fillChild : fillChildren)
                {
                    fillChild->rect.w = fillChild->minBounds.x;
                }
            }
            else
            {
                int remainingWidth = availableWidth - totalFillMinWidth;
                int widthPerFillChild = remainingWidth / numFillChildren;
                int leftoverWidth = remainingWidth % numFillChildren;

                for (size_t i = 0; i < fillChildren.size(); i++)
                {
                    fillChildren[i]->rect.w = fillChildren[i]->minBounds.x + widthPerFillChild + (i < leftoverWidth ? 1 : 0);
                }
            }
        }
    }
    else if (type == NodeType::VSPLITTER)
    {
        int totalFixedHeight = 0;
        int totalFillMinHeight = 0;
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
                totalFillMinHeight += child->minBounds.y;
                numFillChildren++;
            }
        }

        int availableHeight = rect.h - totalFixedHeight;

        if (numFillChildren > 0)
        {
            if (availableHeight < totalFillMinHeight)
            {
                for (Node* fillChild : fillChildren)
                {
                    fillChild->rect.h = fillChild->minBounds.y;
                }
            }
            else
            {
                int remainingHeight = availableHeight - totalFillMinHeight;
                int heightPerFillChild = remainingHeight / numFillChildren;
                int leftoverHeight = remainingHeight % numFillChildren;

                for (size_t i = 0; i < fillChildren.size(); i++)
                {
                    fillChildren[i]->rect.h = fillChildren[i]->minBounds.y + heightPerFillChild + (i < leftoverHeight ? 1 : 0);
                }
            }
        }
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

void SableUI::SplitterNode::Update()
{
    std::vector<int> segments;

    for (SableUI::Node* child : children)
    {
        if (type == NodeType::HSPLITTER)
        {
            segments.push_back(child->rect.x - rect.x);
        }
        if (type == NodeType::VSPLITTER)
		{
			segments.push_back(child->rect.y - rect.y);
		}
	}
    
    m_drawable.Update(rect, m_bColour, type, m_bSize, segments);
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
SableUI::BaseNode::BaseNode(Node* parent, Renderer* renderer) : Node(parent, renderer)
{
    type = NodeType::BASE;
    minBounds = { 50, 50 };
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

}