#pragma once
#include "SBUI_Node.h"
#include "SBUI_Utils.h"
#include "SBUI_Renderer.h"

struct SableUI_node;

class BaseComponent
{
public:
	BaseComponent(SableUI_node* parent) : parent(parent) {}

	virtual void Render() {}
	virtual void UpdateDrawable() {}
	SableUI_node* GetParent() { return parent; }
	void SetParent(SableUI_node* n) { parent = n; }

protected:
	SableUI_node* parent = nullptr;
};

class DefaultComponent : public BaseComponent
{
public:
	DefaultComponent(SableUI::colour colour = SableUI::colour(255, 32, 32, 32), 
		SableUI_node* parent = nullptr) : BaseComponent(parent), colour(colour) {}

	void Render() override;
	void UpdateDrawable() override;

private:
	SableUI::colour colour;
	Drawable::Rect drawable;
};

class SplitterComponent : public BaseComponent
{
public:
	SplitterComponent(SableUI::colour bColour = SableUI::colour(255, 51, 51, 51),
		SableUI_node* parent = nullptr) : BaseComponent(parent), bColour(bColour) {}

	void Render() override;
	void UpdateDrawable() override;
	
private:
	SableUI::colour bColour;
	Drawable::bSplitter drawable;
};