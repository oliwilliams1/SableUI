#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>
#include <memory>

#include "SBUI_Component.h"

constexpr uint16_t UNDEF = 0xFFFF;

class BaseComponent;

enum class NodeType
{
    ROOTNODE  = 0x00,
    COMPONENT = 0x01,
    VSPLITTER = 0x02,
    HSPLITTER = 0x03
};

struct SBUI_node
{
    std::string name = "Unnamed Node";
    NodeType type = NodeType::COMPONENT;

    uint16_t id  = UNDEF;
    uint16_t wPx = UNDEF;
    uint16_t hPx = UNDEF;
    uint16_t xPx = UNDEF;
    uint16_t yPx = UNDEF;
    float wFac   = -1.0f;
    float hFac   = -1.0f;

    SBUI_node* parent  = nullptr;
    std::vector<SBUI_node*> children;

    std::unique_ptr<BaseComponent> component;
    
    SBUI_node(NodeType type, SBUI_node* parent, const std::string& name, int id);
};

void SetupRootNode(SBUI_node* root, uint16_t wPx, uint16_t hPx);