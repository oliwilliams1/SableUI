#pragma once
#include <SableUI/component.h>
#include <SableUI/renderer.h>
#include <SableUI/window.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>

namespace SableUI
{
	class TextField : public BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
		void OnUpdatePostLayout(const UIEventContext& ctx) override;
		void InitData(Window* window)
			{ m_window = window; };

	private:
		State<SableString> textVal{ this, U"" };
		State<bool> isFocused{ this, false };
		State<int> cursorPos{ this, 0 };
		State<int> initialCursorPos{ this, -1 };
		
		bool queueInitialised = false;
		CustomTargetQueue queue;
		Window* m_window = nullptr;
	};
}