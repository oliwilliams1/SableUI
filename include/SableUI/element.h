#pragma once
#include "SableUI/utils.h"

class BaseElement
{
public:
	BaseElement() {};
	virtual void Render() {};

protected:
	SableUI::rect r = { 0, 0, 0, 0 };
	SableUI::colour bgColour = { 255, 0, 0, 0 };
};

class Listbox : public BaseElement
{
public:
	Listbox() {};
	void Render() override {};
};