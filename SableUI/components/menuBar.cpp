#include <string>
#include <SableUI/components/menuBar.h>
#include <SableUI/component.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>
using namespace SableUI;

MenuBar::MenuBar() : BaseComponent()
{
	m_menuItems["File"] = { "New", "Open", "Save", "Save As...", "Exit" };
	m_menuItems["Edit"] = { "Undo", "Redo", "Cut", "Copy", "Paste" };
	m_menuItems["View"] = { "Zoom In", "Zoom Out", "Reset Zoom" };
	m_menuItems["Help"] = { "About" };
}
void MenuBar::Layout()
{
	if (window == nullptr)
	{
		Text("Window is not set, call use, create MenuBar via PanelGainRef() then call SetWindow() on the reference",
			centerY textColour(255, 0, 0));
		return;
	}

	Div(left_right w_fill h_fill bg(51, 51, 51) ID("menu-bar"))
	{
		for (const auto& pair : m_menuItems)
		{
			DrawMenuBarItem(pair.first);
		}
	}
}

void MenuBar::OnUpdate(const SableUI::UIEventContext& ctx)
{
	if (window == nullptr) return;

	if (activeMenu.get().size() > 0 && ctx.mousePressed[SABLE_MOUSE_BUTTON_LEFT])
	{
		SableUI::ElementInfo menuBarInfo = window.get()->GetElementInfoById("menu-bar");
		SableUI::ElementInfo dropdownInfo = window.get()->GetElementInfoById("menu-dropdown");

		bool mouseOverMenuBar = RectBoundingBox(menuBarInfo.rect, ctx.mousePos);
		bool mouseOverDropdown = RectBoundingBox(dropdownInfo.rect, ctx.mousePos);

		//if (!mouseOverMenuBar && !mouseOverDropdown)
		//{
		//	activeMenu.set("");
		//	RemoveQueueFromContext(queue);
		//}
	}

	if (activeMenu.get().size() > 0)
	{
		DrawDropdownMenu();
	}
}

void MenuBar::DrawMenuBarItem(const std::string& text)
{
	bool isSelected = (activeMenu.get() == text);

	Div(p(2) h_fill w_fit ID(text) rounded(4)
		bg(isSelected ? rgb(64, 64, 64) : rgb(51, 51, 51))

		onHover([this, text]() {
			if (activeMenu.get().size() > 0 && activeMenu.get() != text)
			{
				activeMenu.set(text);
			}
		})
	)
	{
		Text(text, justify_center w_fit px(4)
			onClick([this, text]() {
				if (activeMenu.get() == text) {
					activeMenu.set("");
					//RemoveQueueFromContext(queue);
				}
				else {
					activeMenu.set(text);
				}
			})
		);
	}
}

void MenuBar::DrawDropdownMenu()
{
	if (window == nullptr) return;
	SableUI::ElementInfo elInfo = window.get()->GetElementInfoById(activeMenu);

	//UseCustomLayoutContext(queue, window.get(), window.get()->GetSurface(),
	//	absolutePos(elInfo.rect.x, elInfo.rect.y + elInfo.rect.height + 2)
	//	ID("menu-dropdown")
	//)
	//{
	//	Div(bg(45, 45, 45) p(5) minW(150) h_fit rounded(4) up_down)
	//	{
	//		auto it = m_menuItems.find(std::string(activeMenu.get()));
	//		if (it != m_menuItems.end())
	//		{
	//			for (const auto& item : it->second)
	//			{
	//				Div(w_fill bg(45, 45, 45) h_fit p(2) rounded(4))
	//				{
	//					Text(item, px(4)
	//						onClick([this, item]() {
	//							activeMenu.set("");
	//							RemoveQueueFromContext(queue);
	//						})
	//					);
	//				}
	//			}
	//		}
	//		else
	//		{
	//			Text(SableString::Format("No items for %s", std::string(activeMenu.get()).c_str()), justify_center);
	//		}
	//	}
	//}
}