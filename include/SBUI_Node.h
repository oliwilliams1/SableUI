#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <memory>

#include "SBUI_Component.h"
#include "SBUI_Utils.h"
#include "SBUI_Renderer.h"

constexpr uint16_t UNDEF = 0xFFFF;

class BaseComponent;

enum class NodeType
{
    ROOTNODE  = 0x00,
    COMPONENT = 0x01,
    VSPLITTER = 0x02,
    HSPLITTER = 0x03
};

struct SbUI_node
{
    std::string name = "";
    NodeType type = NodeType::COMPONENT;

    uint16_t index = UNDEF;
    SableUI::rect rect = { UNDEF, UNDEF, UNDEF, UNDEF };
    SableUI::vec2 scaleFac = { -1.0f, -1.0f };

    SbUI_node* parent  = nullptr;
    std::vector<SbUI_node*> children;
    std::unique_ptr<BaseComponent> component;
    
    SbUI_node(NodeType type, SbUI_node* parent, const std::string& name);
};

void SetupRootNode(SbUI_node* root, uint16_t wPx, uint16_t hPx);