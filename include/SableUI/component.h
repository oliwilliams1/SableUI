#pragma once
#include "SableUI/node.h"
#include "SableUI/renderer.h"
#include "SableUI/element.h"

struct SableUI_node;

class BaseComponent
{
public:
	BaseComponent(SableUI_node* parent) : parentNode(parent) {};

	virtual void Render() = 0;
	virtual void UpdateDrawable() = 0;
	SableUI_node* GetParent() { return parentNode; }
	void SetParent(SableUI_node* n) { parentNode = n; }

protected:
	SableUI_node* parentNode = nullptr;
};

class DefaultComponent : public BaseComponent
{
public:
	DefaultComponent(SableUI::colour colour = SableUI::colour(255, 32, 32, 32), 
		SableUI_node* parent = nullptr) : BaseComponent(parent), colour(colour) {}
	
	void AddElement(std::unique_ptr<BaseElement>& e);
	void UpdateElements();
	void Render() override;
	void UpdateDrawable() override;
	void RenderElements();


private:
	std::vector<std::unique_ptr<BaseElement>> elements;
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