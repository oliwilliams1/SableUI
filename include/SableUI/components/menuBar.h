#pragma once
#include <map>
#include <string>
#include <vector>
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/window.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>

namespace SableUI
{
	class MenuBar : public SableUI::BaseComponent
	{
	public:
		MenuBar();

		void Layout() override;
		void OnUpdate(const SableUI::UIEventContext& ctx) override;
		void SetWindow(Window* window) { setWindow(window); };

	private:
		CustomLayoutContext(queue);
		useState(activeMenu, setActiveMenu, SableString, "");
		useState(window, setWindow, SableUI::Window*, nullptr);
		std::map<std::string, std::vector<std::string>> m_menuItems;
	
		void DrawMenuBarItem(const std::string& text);
		void DrawDropdownMenu();
	};
}