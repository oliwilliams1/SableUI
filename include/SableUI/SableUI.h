#pragma once
#include "SableUI/window.h"

namespace SableUI
{
	void SetContext(Window* window);
	SplitterPanel* StartSplitter(PanelType orientation);
	void EndSplitter();

	Panel* AddPanel();

	void SetElementBuilderContext(Renderer* renderer, Element* rootElement);
	Element* StartDiv(const ElementInfo& info = {});
	void EndDiv();
	Element* AddText(const std::u32string& text, const ElementInfo& info = {});
	Element* StartRect(const ElementInfo& info = {});
	void EndRect();
}