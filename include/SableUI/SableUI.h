#pragma once
#include "SableUI/window.h"

namespace SableUI
{
	void SetContext(Window* window);
	SplitterPanel* StartSplitter(PanelType orientation);
	void EndSplitter();

	Panel* AddPanel();
}