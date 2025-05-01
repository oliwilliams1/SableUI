#pragma once
#include <string>
#include "node.h"

namespace SableUI
{
	void CreateWindow(const std::string& title, int width, int height, int x = -1, int y = -1);

	bool PollEvents();
	void Draw();
	void SetMaxFPS(int fps);

	void PrintNodeTree();
	void AddNodeToParent(NodeType type, const std::string& name, SBUI_node* parent);

	SBUI_node* GetRoot();
	SBUI_node* FindNodeByName(const std::string& name);

	void Destroy();
}