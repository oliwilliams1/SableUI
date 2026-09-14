#pragma once
#include <SableUI/core/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <functional>

namespace SableUI
{
	class ButtonComponent : public BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIUpdateContext& ctx) override;

		void Init(const SableString& label,
			std::function<void()> callback,
			const ElementInfo& info);

	private:
		ElementInfo info;
		SableString label = "Button";
		std::function<void()> onClickCallback = nullptr;

		State<bool> isPressed{ this, false };
	};
}

// Button component macro
#define Button(label, callback, ...)											\
	ComponentScopedWithStyle(													\
		btn,																	\
		SableUI::ButtonComponent,												\
		this,																	\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__))		\
	)																			\
	btn->Init(label, callback, SableUI::PackStyles(__VA_ARGS__))
