#pragma once
#include <SableUI/core/component.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/window.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/element.h>
#include <functional>

namespace SableUI
{
	struct InputFieldData
	{
		SableString content;
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

		void Init(State<InputFieldData>& data,
			const ElementInfo& info,
			bool multiline = false);

		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;
		void OnUpdatePostLayout(const UIEventContext& ctx) override;

	private:
		ElementInfo info;
		State<InputFieldData>* externalState = nullptr;

		State<int> cursorPos{ this, 0 };
		State<int> initialCursorPos{ this, -1 };
		State<bool> cursorVisible{ this, true };

		bool queueInitialised = false;
		CustomTargetQueue queue;
		Window* m_window = nullptr;
		bool m_multiline = false;

		void ResetCursorBlink();
		void TriggerOnChange();
	};
}