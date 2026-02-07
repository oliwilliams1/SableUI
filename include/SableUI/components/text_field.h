#pragma once
#include <SableUI/core/component.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/core/window.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/element.h>
#include <functional>

namespace SableUI
{
	struct InputFieldData
	{
		SableString content = "";
		SableString placeholder = U"Start typing...";
		std::function<void()> onChange;
		std::function<void()> onSubmit;
		bool isFocused = false;

		bool operator==(const InputFieldData& other) const
		{
			return content == other.content
				&& placeholder == other.placeholder
				&& isFocused == other.isFocused;
		}
	};
	
	class TextFieldComponent : public BaseComponent
	{
	public:
		TextFieldComponent();

		virtual void Init(State<InputFieldData>& data, const ElementInfo& info, bool multiline = false);

		virtual void ContentLeft() {};
		virtual void ContentRight() {};
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
		void OnUpdatePostLayout(const UIEventContext& ctx) override;

	protected:
		ElementInfo info;
		State<InputFieldData>* externalState = nullptr;

	private:

		State<int> cursorPos{ this, 0 };
		State<int> initialCursorPos{ this, -1 };
		State<bool> cursorVisible{ this, true };
		Interval m_cursorBlinkInterval{ this };

		bool queueInitialised = false;
		CustomTargetQueue queue;
		Window* m_window = nullptr;
		bool m_multiline = false;

		void ResetCursorBlink();
		void TriggerOnChange();
	};
}

// Single-line input field
#define InputField(state, ...)													\
	ComponentScopedWithStyle(													\
		field,																	\
		SableUI::TextFieldComponent,											\
		this,																	\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__))		\
	)																			\
	field->Init(state, SableUI::PackStyles(__VA_ARGS__), false)

// Multi-line text field
#define TextField(state, ...)													\
	ComponentScopedWithStyle(													\
		field,																	\
		SableUI::TextFieldComponent,											\
		this,																	\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__))		\
	)																			\
	field->Init(state, SableUI::PackStyles(__VA_ARGS__), true)
