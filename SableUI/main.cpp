#include "SableUI.h"
#include <iostream>

static bool UserModifyNodeGraphViaTerminal()
{
    std::cout << "Add a node (component, vsplitter, or hsplitter) (type 'exit' to quit):" << std::endl;
    std::string command;
    std::getline(std::cin, command);

    if (command == "exit") return false;

    NodeType type;
    if (command == "component") {
        type = NodeType::COMPONENT;
    }
    else if (command == "vsplitter") {
        type = NodeType::VSPLITTER;
    }
    else if (command == "hsplitter") {
        type = NodeType::HSPLITTER;
    }
    else {
        std::cout << "Invalid type! Please enter 'component', 'vsplitter', or 'hsplitter'." << std::endl;
        return true;
    }

    std::cout << "Enter node name: ";
    std::string nodeName;
    std::getline(std::cin, nodeName);

    std::cout << "Enter parent node name: ";
    std::string parentName;
    std::getline(std::cin, parentName);

    SbUI_node* parentNode = SableUI::FindNodeByName(parentName);

    if (parentNode) {
        SableUI::AddNodeToParent(type, nodeName, parentNode->name);
        std::cout << "Node added!" << std::endl;
    }
    else {
        std::cout << "Parent node not found!" << std::endl;
    }
}

int main()
{
	SableUI::CreateWindow("SableUI", 800, 600);
	SableUI::SetMaxFPS(60);

    SableUI::AddNodeToParent(NodeType::HSPLITTER, "H-Splitter 1", "Root Node");
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 1", "H-Splitter 1");
    SableUI::AttachComponentToNode("Component 1", BaseComponent(SbUIcolour(255, 0, 0)));

    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 2", "H-Splitter 1");
    SableUI::AttachComponentToNode("Component 2", BaseComponent(SbUIcolour(0, 255, 0)));

    SableUI::AddNodeToParent(NodeType::VSPLITTER, "V-Splitter 1", "H-Splitter 1");
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 3", "V-Splitter 1");
    SableUI::AttachComponentToNode("Component 3", BaseComponent(SbUIcolour(0, 255, 255)));

    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 4", "V-Splitter 1");
    SableUI::AttachComponentToNode("Component 4", BaseComponent(SbUIcolour(255, 255, 0)));

	SableUI::PrintNodeTree();

	while (SableUI::PollEvents())
	{
        
        // UserModifyNodeGraphViaTerminal();

		SableUI::Draw();
	}

	SableUI::Destroy();
}