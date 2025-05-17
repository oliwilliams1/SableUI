#pragma once
#include <memory>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"

class BaseElement
{
public:
	BaseElement() {};
	BaseElement(SableUI::rect rect) { this->r = rect; };
	BaseElement(SableUI::colour col) { this->bgColour = col; };
	BaseElement(SableUI::rect rect, SableUI::colour col) 
		{ this->r = rect; this->bgColour = col; };

	void Render();
	virtual void AdditionalRender() {};

	void SetRect(const SableUI::rect& rect);

	SableUI::rect r = { 0, 0, 0, 0 };
	SableUI::colour bgColour = { 255, 0, 0, 0 };

private:
	Drawable::Rect bgDrawable;
};

class Listbox : public BaseElement
{
public:
	Listbox() {};
	void AdditionalRender() override {};
};