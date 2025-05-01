#pragma once
#include "SBUI_Utils.h"

class BaseComponent
{
public:
	virtual void Render() = 0;

private:
	SBUIvec3 colour = SBUIvec3(1.0f);
};