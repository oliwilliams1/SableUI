#pragma once
#include <SableUI/scrollContext.h>
#include <SableUI/tabContext.h>
#include <SableUI/component.h>
#include <SableUI/element.h>
#include <SableUI/window.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>

namespace SableUI
{
	class MemoryDebugger : public SableUI::BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
	private:
		State<bool> live{ this, true };
	};

	class LayoutDebugger : public SableUI::BaseComponent
	{
	public:
		LayoutDebugger();
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
		void OnUpdatePostLayout(const UIEventContext& ctx) override;
		void InitData(Window* window) { this->window = window; }

	private:
		State<bool> highlightElements{ this, false };
		State<int> transparency{ this, 0 };
		Window* window = nullptr;
		SableUI::ScrollContext treeScroll;
		SableUI::TabContext tabs;
	};

	class PropertiesPanel : public SableUI::BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

	private:
		State<Rect> selectedRect{ this, {} };
		Ref<ElementInfo> selectedInfo{ this, {} };
		State<Rect> selectedClipRect{ this, {} };
		State<size_t> lastSelectedHash{ this, 0 };
	};
}