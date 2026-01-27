#pragma once
#include <SableUI/components/text_field.h>
#include <SableUI/components/calendar.h>
#include <SableUI/core/component.h>

namespace SableUI
{
	class DatePicker : public TextFieldComponent
	{
	public:
		void ContentRight() override;
		void TextFieldOnUpdatePostLayout(const UIEventContext& ctx) override;

	private:
		State<CalendarContext> calendarCtx{ this, {} };
		State<bool> calendarOpen{ this, false };
	};
}