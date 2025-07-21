#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <memory>

#include "SableUI/component.h"
#include "SableUI/utils.h"
#include "SableUI/renderer.h"

constexpr uint16_t UNDEF = 0xFFFF;

namespace SableUI
{
    class BaseComponent;

    struct Node
    {
        Node(SableUI::NodeType type, Node* parent, const std::string& name, Renderer* renderer);
        ~Node();

        SableUI::Node* AttachSplitter(SableUI::Colour colour, int borderSize);
        SableUI::Node* AttachBase(SableUI::Colour colour);

        std::string name = "";
        SableUI::NodeType type = SableUI::NodeType::COMPONENT;

        uint16_t index = UNDEF;
        SableUI::Rect rect = { UNDEF, UNDEF, UNDEF, UNDEF };

        Node* parent = nullptr;
        std::vector<Node*> children;
        BaseComponent* m_component = nullptr;

        int bSize = 0;
        ivec2 minBounds = {0};

    private:
        Renderer* m_renderer = nullptr;
    };
    
    void SetupRootNode(Node* root, uint16_t wPx, uint16_t hPx);
}