#include <string>
#include <SableUI/components/menuBar.h>
#include <SableUI/component.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>
using namespace SableUI;
using namespace SableUI::Style;

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
			centerY, textColour(255, 0, 0));
		return;
	}

	Div(left_right, w_fill, h_fill, bg(51, 51, 51), id("menu-bar"))
	{
		for (const auto& pair : m_menuItems)
		{
			DrawMenuBarItem(pair.first);
		}
	}
}

void MenuBar::DrawMenuBarItem(const std::string& text)
{
	if (activeMenu.get() == text)
	{
		Div(p(2), h_fill, w_fit, id(text), rounded(4), bg(40, 40, 40))
		{
			Text(text, justify_center, w_fit, px(4));
		}
	}
	else
	{
		Div(p(2), h_fill, w_fit, id(text), rounded(4), hoverBg(rgb(51, 51, 51), rgb(64, 64, 64)))
		{
			Text(text, justify_center, w_fit, px(4), 
				onClick([this, text]()
				{
					activeMenu.set(text); 
				})
			);
		}
	}
}

void MenuBar::DrawDropdownMenu()
{
	if (window == nullptr) return;
	SableUI::Element* el = GetElementById(activeMenu);

	if (!el) return;

	if (!queueInitialised)
	{
		queue.window = window.get();
		queue.target = window.get()->GetSurface();
		queueInitialised = true;
	}

	StartCustomLayoutScope(
		&queue, 
		PackStyles(absolutePos(el->rect.x, el->rect.y + el->rect.height + 2), id("menu-dropdown"))
	);

	Div(bg(45, 45, 45), p(5), minW(150), h_fit, rounded(4), up_down)
	{
		auto it = m_menuItems.find(std::string(activeMenu.get()));
		if (it != m_menuItems.end())
		{
			for (const auto& item : it->second)
			{
				Div(w_fill, bg(45, 45, 45), h_fit, p(2), rounded(4))
				{
					Text(item, px(4));
				}
			}
		}
		else
		{
			Text(SableString::Format("No items for %s", std::string(activeMenu.get()).c_str()), justify_center);
		}
	}

	EndCustomLayoutScope(&queue);
}

void MenuBar::OnUpdate(const SableUI::UIEventContext& ctx)
{
	if (window == nullptr) return;

	if (activeMenu.get().size() > 0 && ctx.mousePressed[SABLE_MOUSE_BUTTON_LEFT])
	{
		SableUI::Element* menuBar = GetElementById("menu-bar");

		if (!menuBar) return;

		bool mouseOverMenuBar = RectBoundingBox(menuBar->rect, ctx.mousePos);

		if (!mouseOverMenuBar)
		{
			activeMenu.set("");
			queue.window->RemoveQueueReference(&queue);
		}
	}

	if (activeMenu.get().size() > 0)
	{
		DrawDropdownMenu();
	}
}
