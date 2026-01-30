#pragma once
#include <SableUI/components/text_field.h>
#include <SableUI/components/calendar.h>
#include <SableUI/core/component.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/SableUI.h>

namespace SableUI
{
	class DatePickerComponent : public TextFieldComponent
	{
	public:
		DatePickerComponent();
		void Init(State<InputFieldData>& data, const ElementInfo& info);
		void ContentRight() override;

		void OnUpdatePostLayout(const UIEventContext& ctx) override;

	private:
		State<CalendarContext> calendarCtx{ this, {} };
	};
}

#define DateField(state, ...)													\
	ComponentScopedWithStyle(													\
		field,																	\
		SableUI::DatePickerComponent,											\
		this,																	\
		SableUI::StripAppearanceStyles(SableUI::PackStyles(__VA_ARGS__))		\
	)																			\
	field->Init(state, SableUI::PackStyles(__VA_ARGS__))