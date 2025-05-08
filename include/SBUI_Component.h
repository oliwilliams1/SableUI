#pragma once
#include "SBUI_Node.h"
#include "SBUI_Utils.h"
#include "SBUI_Renderer.h"

struct SableUI_node;

class BaseComponent
{
public:
	BaseComponent() {}
	BaseComponent(SableUI::colour colour = SableUI::colour(255, 255, 255), float border = 0);

	SableUI_node* parent = nullptr;

	Drawable::rect drawableRect;
	Drawable::rectBorder drawableRectBorder;

	virtual void Render();
	void UpdateDrawable();

	SableUI::Renderer* renderer = nullptr;

private:
	SableUI::colour colour = SableUI::colour(255, 255, 255);
	float border = 0.0f;
};