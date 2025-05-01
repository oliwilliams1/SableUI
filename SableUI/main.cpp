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

    SBUI_node* parentNode = SableUI::FindNodeByName(parentName);

    if (parentNode) {
        SableUI::AddNodeToParent(type, nodeName, parentNode);
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

    SableUI::AddNodeToParent(NodeType::HSPLITTER, "H-Splitter 1", SableUI::FindNodeByName("Root Node"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 1", SableUI::FindNodeByName("H-Splitter 1"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 2", SableUI::FindNodeByName("H-Splitter 1"));

    SableUI::AddNodeToParent(NodeType::VSPLITTER, "V-Splitter 1", SableUI::FindNodeByName("H-Splitter 1"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 3", SableUI::FindNodeByName("V-Splitter 1"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 4", SableUI::FindNodeByName("V-Splitter 1"));

    SableUI::AddNodeToParent(NodeType::HSPLITTER, "H-Splitter 2", SableUI::FindNodeByName("Root Node"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 5", SableUI::FindNodeByName("H-Splitter 2"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 6", SableUI::FindNodeByName("H-Splitter 2"));

    SableUI::AddNodeToParent(NodeType::VSPLITTER, "V-Splitter 2", SableUI::FindNodeByName("H-Splitter 2"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 7", SableUI::FindNodeByName("V-Splitter 2"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 8", SableUI::FindNodeByName("V-Splitter 2"));

    SableUI::AddNodeToParent(NodeType::HSPLITTER, "H-Splitter 3", SableUI::FindNodeByName("V-Splitter 2"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 9", SableUI::FindNodeByName("H-Splitter 3"));
    SableUI::AddNodeToParent(NodeType::COMPONENT, "Component 10", SableUI::FindNodeByName("H-Splitter 3"));


	while (SableUI::PollEvents())
	{
		SableUI::PrintNodeTree();

        UserModifyNodeGraphViaTerminal();

		SableUI::Draw();
	}

	SableUI::Destroy();
}