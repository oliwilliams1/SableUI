#include "SableUI/panel.h"

SableUI::BasePanel::BasePanel(BasePanel* parent, Renderer* renderer) : parent(parent), m_renderer(renderer)
{
    type = PanelType::BASE;
    rect.wType = RectType::FILL;
    rect.hType = RectType::FILL;
}

/* Root node implementation */
SableUI::RootPanel::RootPanel(Renderer* renderer, int w, int h) : BasePanel(nullptr, renderer)
{
    type = PanelType::ROOTNODE;
    rect.w = w;
    rect.h = h;
}

SableUI::RootPanel::~RootPanel()
{
    for (SableUI::BasePanel* child : children)
    {
        delete child;
    }
}

void SableUI::RootPanel::Render()
{
    for (SableUI::BasePanel* child : children)
    {
        child->Render();
    }
}

void SableUI::RootPanel::Recalculate()
{
    for (SableUI::BasePanel* child : children)
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

SableUI::SplitterPanel* SableUI::RootPanel::AddSplitter(PanelType type)
{
    if (children.size() > 0)
    {
        SableUI_Error("Root node cannot have more than one child, dismissing call");
        return nullptr;
    }
    SplitterPanel* node = new SplitterPanel(this, type, m_renderer);
    children.push_back(node);

    Recalculate();
    return node;
}

SableUI::Panel* SableUI::RootPanel::AddPanel()
{
    if (children.size() > 0)
    {
        SableUI_Error("Root node cannot have more than one child, dismissing call");
        return nullptr;
    }
    Panel* node = new Panel(this, m_renderer);
    children.push_back(node);

    Recalculate();
    return node;
}

SableUI::BasePanel* SableUI::BasePanel::FindRoot()
{
    BasePanel* node = this;
    while (node->parent != nullptr) node = node->parent;
    return node;
}

void SableUI::RootPanel::CalculateScales()
{
    SableUI_Error("Method does not exist for root node, use RootNode.Recalculate() instead");
    return;
}

void SableUI::RootPanel::CalculatePositions()
{
    SableUI_Error("Method does not exist for root node, use RootNode.Recalculate() instead");
    return;
}

void SableUI::RootPanel::Resize(int w, int h)
{
    rect.w = w;
    rect.h = h;
}

/* Splitter node implementation */
SableUI::SplitterPanel::SplitterPanel(BasePanel* parent, PanelType type, Renderer* renderer) : BasePanel(parent, renderer)
{
    this->type = type;
}

void SableUI::SplitterPanel::Render()
{
    if (!m_drawableUpToDate) Update();
    for (SableUI::BasePanel* child : children)
    {
        child->Render();
    }

    m_drawable.m_zIndex = 999;
    m_renderer->Draw(&m_drawable);
}

SableUI::SplitterPanel* SableUI::SplitterPanel::AddSplitter(PanelType type)
{
    SplitterPanel* node = new SplitterPanel(this, type, m_renderer);
    children.push_back(node);

    FindRoot()->Recalculate();
    return node;
}

SableUI::Panel* SableUI::SplitterPanel::AddPanel()
{
    Panel* node = new Panel(this, m_renderer);
    children.push_back(node);

    FindRoot()->Recalculate();
    return node;
}

void SableUI::SplitterPanel::CalculateScales()
{
    if (children.empty()) return;

    m_drawableUpToDate = false;

    if (type == PanelType::HORIZONTAL)
    {
        int totalFixedWidth = 0;
        int numFillChildren = 0;
        std::vector<BasePanel*> fillChildren;

        for (BasePanel* child : children)
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
    else if (type == PanelType::VERTICAL)
    {
        int totalFixedHeight = 0;
        int numFillChildren = 0;
        std::vector<BasePanel*> fillChildren;

        for (BasePanel* child : children)
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

    for (BasePanel* child : children)
    {
        child->CalculateScales();
    }
}


void SableUI::SplitterPanel::CalculatePositions()
{
    if (children.empty()) return;

    m_drawableUpToDate = false;
    bool toUpdate = false;

    vec2 cursor = { rect.x, rect.y };

    if (type == PanelType::HORIZONTAL)
    {
        for (BasePanel* child : children)
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
    else if (type == PanelType::VERTICAL)
    {
        for (BasePanel* child : children)
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

    for (BasePanel* child : children)
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

void SableUI::SplitterPanel::CalculateMinBounds()
{
    if (children.empty())
    {
        minBounds = { 20, 20 };
        return;
    }

    if (type == PanelType::HORIZONTAL)
    {
        int totalWidth = 0;
        int maxHeight = 0;

        for (BasePanel* child : children)
        {
            child->CalculateMinBounds();
            totalWidth += child->rect.wType == SableUI::FILL ? child->minBounds.x : child->rect.w;
            maxHeight = std::max(maxHeight, child->rect.hType == SableUI::FILL ? child->minBounds.y : child->rect.h);
        }

        minBounds = { totalWidth, maxHeight };
    }
    else if (type == PanelType::VERTICAL)
    {
        int totalHeight = 0;
        int maxWidth = 0;

        for (BasePanel* child : children)
        {
            child->CalculateMinBounds();
            totalHeight += child->rect.hType == SableUI::FILL ? child->minBounds.y : child->rect.h;
            maxWidth = std::max(maxWidth, child->rect.wType == SableUI::FILL ? child->minBounds.x : child->rect.w);
        }

        minBounds = { maxWidth, totalHeight };
    }
}

void SableUI::SplitterPanel::Update()
{
    std::vector<int> segments;

    for (SableUI::BasePanel* child : children)
    {
        child->Update();

        if (type == PanelType::HORIZONTAL)
        {
            segments.push_back(child->rect.x - rect.x);
        }
        if (type == PanelType::VERTICAL)
		{
			segments.push_back(child->rect.y - rect.y);
		}
	}
    
    m_drawable.Update(rect, m_bColour, type, bSize, segments);
    m_drawableUpToDate = true;
    Render();
}

SableUI::SplitterPanel::~SplitterPanel()
{
    for (BasePanel* child : children)
    {
        delete child;
    }
    children.clear();
}

/* Base Node Implementation */
static int s_ctr = 0;
SableUI::Panel::Panel(BasePanel* parent, Renderer* renderer) : BasePanel(parent, renderer)
{
    type = PanelType::BASE;
}

SableUI::SplitterPanel* SableUI::Panel::AddSplitter(PanelType type)
{
    SableUI_Error("Base node cannot have any children, skipping call");
    return nullptr;
}

SableUI::Panel* SableUI::Panel::AddPanel()
{
    SableUI_Error("Base node cannot have any children, skipping call");
    return nullptr;
}

void SableUI::Panel::Update()
{
    if (m_component == nullptr)
    {
        m_component = new BaseComponent();
        m_component->BackendInitialise(m_renderer);
    }

    SableUI::Rect realRect = rect;
    
    if (auto* splitter = dynamic_cast<SplitterPanel*>(parent); splitter != nullptr)
    {
        realRect.x += splitter->bSize;
		realRect.y += splitter->bSize;
        realRect.w -= splitter->bSize * 2;
        realRect.h -= splitter->bSize * 2;
    }

    m_component->GetRootElement()->SetRect(realRect);
}

void SableUI::Panel::Render()
{
    m_component->GetRootElement()->Render();
}