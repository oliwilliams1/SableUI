#pragma once
#include <SableUI/scrollContext.h>
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <SableUI/window.h>
#include <SableUI/events.h>
#include <SableUI/panel.h>
#include <unordered_set>
#include <SableUI/tabContext.h>

namespace SableUI
{
	class MemoryDebugger : public SableUI::BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
	private:
		useState(live, setLive, bool, true);
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
		useState(highlightElements, setHighlightElements, bool, false);
		useState(transparency, setTransparency, int, 0);
		Window* window = nullptr;
		SableUI::ScrollContext treeScroll;
		SableUI::TabContext tabs;
	};

	class ElementTreeView : public SableUI::BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
		void InitData(Window* window, bool highlightElements, int transparency);

	private:
		void DrawElementTree(Element* element, int depth, int& line, size_t parentHash);
		void DrawPanelTree(BasePanel* panel, int depth, int& line, size_t parentHash);
		size_t ComputeHash(const void* ptr, size_t parentHash) const;

		useState(expandedNodes, setExpandedNodes, std::unordered_set<size_t>, {});

		Window* window = nullptr;
		bool highlightElements = false;
		int transparency = 0;
		CustomLayoutContext(queue);

		size_t lastDrawnHoveredHash = 0;
	};

	class PropertiesPanel : public SableUI::BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

	private:
		useState(selectedRect, setSelectedRect, Rect, {});
		useRef(selectedInfo, ElementInfo, {});
		useState(selectedClipRect, setSelectedClipRect, Rect, {});
		useState(lastSelectedHash, setLastSelectedHash, size_t, 0);
	};
}