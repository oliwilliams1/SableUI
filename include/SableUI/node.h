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
    struct Node
    {
        Node(Node* parent, Renderer* renderer, const char* name);
        virtual ~Node() = default;
        virtual void Render() = 0;
        virtual void Recalculate() = 0;
        virtual void DebugDrawBounds() = 0;
        virtual void AddChild(Node* node) = 0;

        Node* parent = nullptr;
        SableUI::Rect rect = { 0, 0, 0, 0 };
        ivec2 minBounds = { 0 };
        std::string name = "";
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
        void DebugDrawBounds() override;
        void AddChild(Node* node) override;
    };

    struct SplitterNode : public Node
    {
        SplitterNode(Node* parent, NodeType type, Renderer* renderer, const char* name) : Node(parent, renderer, name) {};
        ~SplitterNode();

        void Render() override;
        void Recalculate() override;
        void DebugDrawBounds() override;
        void AddChild(Node* node) override;

    private:
        int bSize = 0;
    };

    struct BaseNode : public Node
    {
        BaseNode(Node* parent, Renderer* renderer, const char* name);

        void Render() override {};
        void Recalculate() override {};
        void DebugDrawBounds() override;
        void AddChild(Node* node) override;
    };   
}