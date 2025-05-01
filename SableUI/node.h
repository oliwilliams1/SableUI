#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstdint>

constexpr uint16_t UNDEF = 0xFFFF;

enum class NodeType
{
    ROOTNODE  = 0x00,
    COMPONENT = 0x01,
    VSPLITTER = 0x02,
    HSPLITTER = 0x03
};


struct Node
{
    std::string name = "Unnamed Node";
    NodeType type = NodeType::COMPONENT;
    uint16_t id   = UNDEF;
    uint16_t wPx  = UNDEF;
    uint16_t hPx  = UNDEF;
    float wFac    = -1.0f;
    float hFac    = -1.0f;
    Node* parent  = nullptr;
    std::vector<Node*> children;

    Node(NodeType type, Node* parent, const std::string& name, int id);
};

void SetupRootNode(Node* root, uint16_t wPx, uint16_t hPx);