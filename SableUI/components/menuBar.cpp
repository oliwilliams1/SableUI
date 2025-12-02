#include <string>
#include <SableUI/components/menuBar.h>
#include <SableUI/component.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>

MenuBar::MenuBar(SableUI::Window* window) : BaseComponent(), m_window(window)
{
	m_menuItems["File"] = { "New", "Open", "Save", "Save As...", "Exit" };
	m_menuItems["Edit"] = { "Undo", "Redo", "Cut", "Copy", "Paste" };
	m_menuItems["View"] = { "Zoom In", "Zoom Out", "Reset Zoom" };
	m_menuItems["Help"] = { "About" };
}
void MenuBar::Layout()
{
	Div(left_right h_fill bg(32, 32, 32) ID("menu-bar"))
	{
		for (const auto& pair : m_menuItems)
		{
			DrawMenuBarItem(pair.first);
		}
	}
}

void MenuBar::OnUpdate(const SableUI::UIEventContext& ctx)
{
	if (activeMenu.size() > 0 && ctx.mousePressed[SABLE_MOUSE_BUTTON_LEFT])
	{
		SableUI::ElementInfo menuBarInfo = m_window->GetElementInfoById("menu-bar");
		SableUI::ElementInfo dropdownInfo = m_window->GetElementInfoById("menu-dropdown");

		bool mouseOverMenuBar = RectBoundingBox(menuBarInfo.rect, ctx.mousePos);
		bool mouseOverDropdown = RectBoundingBox(dropdownInfo.rect, ctx.mousePos);

		if (!mouseOverMenuBar && !mouseOverDropdown)
		{
			setActiveMenu("");
			RemoveQueueFromContext(queue);
		}
	}

	if (activeMenu.size() > 0)
	{
		DrawDropdownMenu();
	}
}

void MenuBar::DrawMenuBarItem(const std::string& text)
{
	bool isSelected = (activeMenu == text);

	Div(p(2) h_fill w_fit ID(text) rounded(4)
		bg(isSelected ? rgb(64, 64, 64) : rgb(32, 32, 32))

		onHover([this, text]() {
			if (activeMenu.size() > 0 && activeMenu != text)
			{
				setActiveMenu(text);
			}
			})
	)
	{
		Text(text, justify_center w_fit px(4)
			onClick([this, text]() {
				if (activeMenu == text) {
					setActiveMenu("");
					RemoveQueueFromContext(queue);
				}
				else {
					setActiveMenu(text);
				}
			})
		);
	}
}

void MenuBar::DrawDropdownMenu()
{
	SableUI::ElementInfo elInfo = m_window->GetElementInfoById(activeMenu);

	UseCustomLayoutContext(queue, m_window, m_window->GetSurface(),
		absolute(elInfo.rect.x, elInfo.rect.y + elInfo.rect.height + 2)
		ID("menu-dropdown")
	)
	{
		Div(bg(45, 45, 45) p(5) minW(150) h_fit rounded(4) up_down)
		{
			auto it = m_menuItems.find(std::string(activeMenu));
			if (it != m_menuItems.end())
			{
				for (const auto& item : it->second)
				{
					Div(w_fill bg(45, 45, 45) h_fit p(2) rounded(4))
					{
						Text(item, px(4)
							onClick([this, item]() {
								setActiveMenu("");
								RemoveQueueFromContext(queue);
							})
						);
					}
				}
			}
			else
			{
				Text(SableString::Format("No items for %s", std::string(activeMenu).c_str()), justify_center);
			}
		}
	}
}