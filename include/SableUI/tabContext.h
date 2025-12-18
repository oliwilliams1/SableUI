#pragma once
#include <SableUI/element.h>
#include <vector>
#include <string>

namespace SableUI
{
	struct TabContext
	{
		int activeTab = 0;
		std::vector<std::string> tabs;
		bool changed = false;

		TabContext& Add(const std::string& name);
	};

	void RenderTabHeader(TabContext& ctx, const ElementInfo& style = ElementInfo{});
}