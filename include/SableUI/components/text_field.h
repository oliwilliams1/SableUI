#pragma once
#include <SableUI/core/component.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/window.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>

namespace SableUI
{
	class TextField : public BaseComponent
	{
	public:
		TextField();

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
		State<bool> cursorVisible{ this, true };
		
		bool queueInitialised = false;
		CustomTargetQueue queue;
		Window* m_window = nullptr;

		void ResetCursorBlink();
	};
}