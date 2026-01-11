#pragma once
#include <SableUI/core/component.h>
#include <SableUI/SableUI.h>
#include <functional>

namespace SableUI
{
	class Button : public BaseComponent
	{
	public:
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

		void Init(const SableString& label,
			std::function<void()> callback,
			const ElementInfo& info);

	private:
		ElementInfo info;
		State<SableString> label{ this, "Button" };
		State<bool> isPressed{ this, false };
		Ref<std::function<void()>> onClickCallback{ this, nullptr };
	};
}

// Button component
#define Button(label, callback, ...)											\
	ComponentScopedWithStyle(													\
		SableUI::Button,														\
		btn,																	\
		this,																	\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__))		\
	)																			\
	btn->Init(label, callback, SableUI::PackStyles(__VA_ARGS__))
