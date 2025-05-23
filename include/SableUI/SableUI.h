#pragma once
#define SDL_MAIN_HANDLED
#include <string>
#include "SableUI/node.h"

namespace SableUI
{
	void SBCreateWindow(const std::string& title, int width, int height, int x = -1, int y = -1);

	bool PollEvents();
	void Draw();
	void SetMaxFPS(int fps);

	void PrintNodeTree();
	void SetupSplitter(const std::string& name, float bSize);
	void AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName);
	void AttachComponentToNode(const std::string& nodeName, std::unique_ptr<BaseComponent> component);
	void AddElementToComponent(const std::string& nodeName, const ElementInfo& info);

	Node* GetRoot();
	Node* FindNodeByName(const std::string& name);

	void Destroy();

	void OpenUIFile(const std::string& path);
	void RecalculateNodes();

	void CalculateNodePositions(Node* node = nullptr);
}