#pragma once
#define SDL_MAIN_HANDLED
#include <string>
#include "SBUI_Node.h"

namespace SableUI
{
	void CreateWindow(const std::string& title, int width, int height, int x = -1, int y = -1);

	bool PollEvents();
	void Draw();
	void SetMaxFPS(int fps);

	void PrintNodeTree();
	void AddNodeToParent(NodeType type, const std::string& name, const std::string& parentName);
	void AttachComponentToNode(const std::string& nodeName, const BaseComponent& component);

	SableUI_node* GetRoot();
	SableUI_node* FindNodeByName(const std::string& name);

	void Destroy();

	void OpenUIFile(const std::string& path);

	void CalculateNodeDimensions(SableUI_node* node = nullptr);
}