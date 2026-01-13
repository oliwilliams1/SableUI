#include <SableUI/SableUI.h>
#include <SableUI/core/context_menu.h>
#include <SableUI/utils/console.h>
#include <SableUI/core/component.h>
#include <SableUI/SableUI.h>
#include <vector>

using namespace SableUI::Style;

class ContextMenuComponent : public SableUI::BaseComponent
{
public:
	void Init(const std::vector<SableUI::ContextMenuItem>& items)
	{
		m_items = items;
	}

	void Layout() override
	{
		Text("Hello!");
		for (const auto& item : m_items)
		{
			switch (item.type)
			{
			case SableUI::ContextMenuItemType::Normal:
				Text(item.text);
				break;
			case SableUI::ContextMenuItemType::Seperator:
				SplitterHorizontal();
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
	if (m_isOpen)
	{
		SableUI_Warn("Cannot add items to an open context menu");
		return m_items.back();
	}

	m_items.push_back({ ContextMenuItemType::Normal, text, callback });
	return m_items.back();
}

SableUI::ContextMenuItem& SableUI::ContextMenu::AddSeperator()
{
	if (m_isOpen)
	{
		SableUI_Warn("Cannot add items to an open context menu");
		return m_items.back();
	}

	m_items.push_back({ ContextMenuItemType::Seperator, {}, {} });
	return m_items.back();
}

void SableUI::ContextMenu::Update(const UIEventContext& ctx)
{
	if (m_showQueried && !m_isOpen)
	{
		p_show(ctx);

		m_isOpen = true;
		m_showQueried = false;
		return;
	}

	if (m_hideQueried && m_isOpen)
	{
		p_hide(ctx);

		m_isOpen = false;
		m_hideQueried = false;
		return;
	}

	if (m_isOpen && ctx.mousePressed.test(SABLE_MOUSE_BUTTON_LEFT))
	{
		p_hide(ctx);
		m_isOpen = false;
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
	int totalHeight = 0;
	for (const auto& item : m_items)
	{
		switch (item.type)
		{
		case ContextMenuItemType::Normal:
			totalHeight += 20;
			break;
		case ContextMenuItemType::Seperator:
			totalHeight += 5;
			break;
		default:
			SableUI_Error("Context menu has unknown item type");
		}
	}

	return totalHeight;
}

void SableUI::ContextMenu::p_show(const UIEventContext& ctx)
{
	m_rect.x = ctx.mousePos.x;
	m_rect.y = ctx.mousePos.y;
	m_rect.w = 200;
	m_rect.h = GetHeight();

	//CreateFloatingPanel("context menu", "ContextMenuComponent", m_rect);
	FloatingPanelScoped(ref, ContextMenuComponent, "context menu", m_rect)
	{
		ref->Init(m_items);
	}
}

void SableUI::ContextMenu::p_hide(const UIEventContext& ctx)
{
	QueueDestroyFloatingPanel("context menu");
}
