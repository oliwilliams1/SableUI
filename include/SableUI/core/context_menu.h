#pragma once
#include <SableUI/utils/utils.h>
#include <SableUI/core/events.h>
#include <functional>
#include <vector>

namespace SableUI
{
	enum class ContextMenuItemType
	{
		Undef,
		Normal,
		Seperator
	};

	struct ContextMenuItem
	{
		ContextMenuItemType type;
		SableString text;
		std::function<void()> callback;

		ContextMenuItem& AddIcon(const SableString& icon) { return *this; };
		ContextMenuItem& AddShortcut(const SableString& shortcut) { return *this; };
	};

	struct ContextMenu
	{
		ContextMenuItem& AddItem(const SableString& text, std::function<void()> callback);
		ContextMenuItem& AddSeperator();

		void show();
		void hide();
		
		void Update(const UIEventContext& ctx);


	private:
		int GetHeight();
		void p_show(const UIEventContext& ctx);
		void p_hide(const UIEventContext& ctx);

		Rect m_rect;
		bool m_showQueried = false;
		bool m_hideQueried = false;
		std::vector<ContextMenuItem> m_items;
	};
}