#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <memory>

#include "SableUI/component.h"
#include "SableUI/utils.h"
#include "SableUI/renderer.h"

namespace SableUI
{
    struct SplitterNode;
    struct BaseNode;

    struct Node
    {
        Node(Node* parent, Renderer* renderer);
        virtual ~Node() = default;
        virtual void Render() = 0;
        virtual void Recalculate() {};

        virtual SplitterNode* AddSplitter(NodeType type) = 0;
        virtual BaseNode* AddBaseNode() = 0;

        virtual void CalculateScales() {};
        virtual void CalculatePositions() {};

        Node* parent = nullptr;
        SableUI::Rect rect = { 0, 0, 0, 0 };
        ivec2 minBounds = { 0 };
        NodeType type = NodeType::UNDEF;
        std::vector<Node*> children;

    protected:
		Renderer* m_renderer = nullptr;
    };

    struct RootNode : public Node
    {
        RootNode(Renderer* renderer, int w, int h);
        ~RootNode();

        void Resize(int w, int h);
        void Render() override;
        void Recalculate() override;

        SplitterNode* AddSplitter(NodeType type) override;
        BaseNode* AddBaseNode() override;

        void CalculateScales() override;
        void CalculatePositions() override;
    };

    struct SplitterNode : public Node
    {
        SplitterNode(Node* parent, NodeType type, Renderer* renderer);
        ~SplitterNode();

        void Render() override;

        SplitterNode* AddSplitter(NodeType type) override;
        BaseNode* AddBaseNode() override;

        void CalculateScales() override;
        void CalculatePositions() override;

    private:
        void UpdateDrawable();
        DrawableSplitter* m_drawable = nullptr;
        bool m_drawableUpToDate = false;

        int m_bSize = 1;
        Colour m_bColour = { 51, 51, 51 };
    };

    struct BaseNode : public Node
    {
        BaseNode(Node* parent, Renderer* renderer);

        void Render() override {};
        SplitterNode* AddSplitter(NodeType type) override;
        BaseNode* AddBaseNode() override;
    };   
}