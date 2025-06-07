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
        std::string name = "";
        SableUI::NodeType type = SableUI::NodeType::COMPONENT;

        uint16_t index = UNDEF;
        SableUI::Rect rect = { UNDEF, UNDEF, UNDEF, UNDEF };

        Node* parent = nullptr;
        std::vector<Node*> children;
        std::unique_ptr<SableUI::BaseComponent> component;

        float bSize = 0.0f;

        Node(SableUI::NodeType type, Node* parent, const std::string& name);
    };
    
    void SetupRootNode(Node* root, uint16_t wPx, uint16_t hPx);
}