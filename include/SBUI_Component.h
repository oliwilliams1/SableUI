#pragma once
#include "SBUI_Node.h"
#include "SBUI_Utils.h"

struct SBUI_node;

class BaseComponent
{
public:
	BaseComponent() {}
	BaseComponent(SBUIvec3 colour) { this->colour = colour; }

	SBUI_node* parent = nullptr;

	virtual void Render();

private:
	SBUIvec3 colour = SBUIvec3(1.0f);
};