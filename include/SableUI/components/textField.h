#pragma once
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
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
		void OnPostLayoutUpdate(const UIEventContext& ctx) override;
		void SetWindow(Window* window) { m_window = window; };

	private:
		CustomLayoutContext(queue);
		useState(textVal, setTextVal, SableString, U"");
		useState(isFocused, setIsFocused, bool, false);
		Window* m_window = nullptr;
	};
}