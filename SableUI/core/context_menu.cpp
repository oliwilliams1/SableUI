#include <SableUI/SableUI.h>
#include <SableUI/core/context_menu.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/component.h>
#include <SableUI/components/button.h>
#include <SableUI/styles/styles.h>
#include <vector>
#include <algorithm>

using namespace SableUI::Style;

class ContextMenuComponent : public SableUI::BaseComponent
{
public:
	void Init(const std::vector<SableUI::ContextMenuItem>& items)
	{
		m_items = items;
		MarkDirty();
	}

	void Layout() override
	{
		for (const auto& item : m_items)
		{
			switch (item.type)
			{
			case SableUI::ContextMenuItemType::Normal:
				Button(item.text, [cb = item.callback]() {
					cb();
					SableUI::QueueDestroyFloatingPanel("context menu");
				}, size_sm, w_fill, justify_left, mb(2));
				break;
			case SableUI::ContextMenuItemType::Seperator:
				SableUI::AddRect(SableUI::PackStyles(mx(2), mb(4), mt(2), h(1), w_fill, bg(70, 70, 70)));
				break;
			default:
				SableUI_Error("Context menu has unknown item type");
			}
		}
	}

private:
	std::vector<SableUI::ContextMenuItem> m_items;
};

SableUI::ContextMenuItem& SableUI::ContextMenu::AddItem(const SableString& text, std::function<void()> callback)
{
	if (IsFloatingPanelActive("context menu"))
	{
		SableUI_Warn("Cannot add items to an open context menu");
		return m_items.back();
	}

	m_items.push_back({ ContextMenuItemType::Normal, text, callback });
	return m_items.back();
}

SableUI::ContextMenuItem& SableUI::ContextMenu::AddSeperator()
{
	if (IsFloatingPanelActive("context menu"))
	{
		SableUI_Warn("Cannot add items to an open context menu");
		return m_items.back();
	}

	m_items.push_back({ ContextMenuItemType::Seperator, {}, {} });
	return m_items.back();
}

void SableUI::ContextMenu::Update(const UIEventContext& ctx)
{
	const bool isOpen = IsFloatingPanelActive("context menu");

	if (m_showQueried)
	{
		if (isOpen)
			p_hide(ctx);

		p_show(ctx);
		m_showQueried = false;
		return;
	}

	if (m_hideQueried && isOpen)
	{
		p_hide(ctx);
		m_hideQueried = false;
		return;
	}

	if (!isOpen)
		return;

	const bool mouseInMenu = RectBoundingBox(m_rect, ctx.mousePos);

	if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
	{
		if (!mouseInMenu)
		{
			p_hide(ctx);
			return;
		}
	}

	if (ctx.mousePressed.test(SABLE_MOUSE_BUTTON_RIGHT))
	{
		p_hide(ctx);
		p_show(ctx);
	}
}


void SableUI::ContextMenu::show()
{
	m_showQueried = true;
}

void SableUI::ContextMenu::hide()
{
	m_hideQueried = true;
}

int SableUI::ContextMenu::GetHeight()
{
	int totalHeight = -2;
	for (const auto& item : m_items)
	{
		switch (item.type)
		{
		case ContextMenuItemType::Normal:
			totalHeight += 22;
			break;
		case ContextMenuItemType::Seperator:
			totalHeight += 8;
			break;
		default:
			SableUI_Error("Context menu has unknown item type");
		}
	}

	return (std::max)(totalHeight, 0);
}

void SableUI::ContextMenu::p_show(const UIEventContext& ctx)
{
	m_rect.x = ctx.mousePos.x;
	m_rect.y = ctx.mousePos.y;
	m_rect.w = 200;
	m_rect.h = GetHeight();

	FloatingPanelScoped(ref, ContextMenuComponent, "context menu", m_rect, rounded(4), bg(51, 51, 51))
	{
		ref->Init(m_items);
	}
}

void SableUI::ContextMenu::p_hide(const UIEventContext& ctx)
{
	QueueDestroyFloatingPanel("context menu");
}
