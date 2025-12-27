#include <string>
#include <SableUI/components/menuBar.h>
#include <SableUI/component.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>
#include <SableUI/styles.h>
#include <SableUI/theme.h>

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
	const Theme& t = GetTheme();

	if (window == nullptr)
	{
		Text("Window is not set, call use, create MenuBar via PanelGainRef() then call SetWindow() on the reference",
			centerY, textColour(t.red));
		return;
	}

	Div(left_right, w_fill, h_fill, bg(t.surface1), id("menu-bar"), gap(4), ml(2))
	{
		for (const auto& pair : m_menuItems)
		{
			DrawMenuBarItem(pair.first, t);
		}
	}
}

void MenuBar::DrawMenuBarItem(const std::string& text, const Theme& t)
{
	if (activeMenu.get() == text)
	{
		Div(p(1), h_fill, w_fit, id(text), rounded(4), bg(t.overlay1))
		{
			Text(text, justify_center, w_fit, px(2));
		}
	}
	else
	{
		Div(p(1), h_fill, w_fit, id(text), rounded(4), hoverBg(t.surface1, t.overlay1))
		{
			Text(text, justify_center, w_fit, px(2), 
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
	const Theme& t = GetTheme();

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

	Div(bg(t.surface1), p(2), minW(150), h_fit, rounded(4), up_down)
	{
		auto it = m_menuItems.find(std::string(activeMenu.get()));
		if (it != m_menuItems.end())
		{
			for (const auto& item : it->second)
			{
				Div(w_fill, h_fit, p(2), rounded(4))
				{
					Text(item, px(2));
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
