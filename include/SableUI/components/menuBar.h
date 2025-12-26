#pragma once
#include <SableUI/component.h>
#include <SableUI/window.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>
#include <string>
#include <vector>
#include <map>

namespace SableUI
{
	class MenuBar : public SableUI::BaseComponent
	{
	public:
		MenuBar();

		void Layout() override;
		void OnUpdate(const SableUI::UIEventContext& ctx) override;
		void SetWindow(Window* p_window) { window.set(p_window); };

	private:
		State<SableString> activeMenu{ this, U"" };
		std::map<std::string, std::vector<std::string>> m_menuItems;

		bool queueInitialised = false;
		CustomTargetQueue queue;
		State<Window*> window{ this, nullptr };
	
		void DrawMenuBarItem(const std::string& text);
		void DrawDropdownMenu();
	};
}