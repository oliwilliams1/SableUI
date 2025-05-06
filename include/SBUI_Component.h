#pragma once
#include "SBUI_Node.h"
#include "SBUI_Utils.h"

struct SableUI_node;

class BaseComponent
{
public:
	BaseComponent() {}
	BaseComponent(SableUI::colour colour) { this->colour = colour; }

	SableUI_node* parent = nullptr;

	virtual void Render();

private:
	SableUI::colour colour = SableUI::colour(255, 255, 255);
};