#pragma once
#include <map>
#include <string>
#include <vector>
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/window.h>

class MenuBar : public SableUI::BaseComponent
{
public:
	MenuBar(SableUI::Window* window);

	void Layout() override;
	void OnUpdate(const SableUI::UIEventContext& ctx) override;

private:
	CustomLayoutContext(queue);
	useState(activeMenu, setActiveMenu, SableString, "");
	
	SableUI::Window* m_window = nullptr;
	std::map<std::string, std::vector<std::string>> m_menuItems;
	
	void DrawMenuBarItem(const std::string& text);
	void DrawDropdownMenu();
};