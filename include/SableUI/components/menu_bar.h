#pragma once
#include <SableUI/core/component.h>
#include <SableUI/core/window.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/renderer.h>
#include <SableUI/styles/theme.h>
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
	
		void DrawMenuBarItem(const std::string& text, const Theme& t);
		void DrawDropdownMenu();
	};
}