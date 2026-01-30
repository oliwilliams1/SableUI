#include <SableUI/components/date_picker.h>
#include <SableUI/components/calendar.h>
#include <SableUI/styles/styles.h>
#include <SableUI/components/button.h>
#include <SableUI/components/text_field.h>
#include <SableUI/core/component.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/styles/theme.h>
#include <SableUI/utils/utils.h>

using namespace SableUI::Style;

SableUI::DatePickerComponent::DatePickerComponent()
{
	const CalendarContext& dataRef = calendarCtx.get();
	
	bool uninitialised =
		dataRef.viewYear == -1
		|| dataRef.viewMonth == -1
		|| dataRef.selectedDay == -1
		|| dataRef.selectedMonth == -1
		|| dataRef.selectedYear == -1;

	if (uninitialised)
		if (InitCalendarToToday(calendarCtx))
			MarkDirty();
}

inline static SableString FormattedDate(const SableUI::CalendarContext& ctx)
{
	return SableString::Format("%02d/%02d/%04d",
		ctx.selectedDay,
		ctx.selectedMonth + 1,
		ctx.selectedYear);
}

static void UpdateTextFieldWithCalendarData(
	SableUI::State<SableUI::InputFieldData>& field,
	SableUI::State<SableUI::CalendarContext> ctx)
{
	SableUI::InputFieldData fieldCopy = field.get();
	fieldCopy.content = FormattedDate(ctx.get());

	if (fieldCopy != field.get())
		field.set(fieldCopy);
}

void SableUI::DatePickerComponent::Init(State<InputFieldData>& data, const ElementInfo& info)
{
	TextFieldComponent::Init(data, info, false);

	UpdateTextFieldWithCalendarData(*externalState, calendarCtx);
}

void SableUI::DatePickerComponent::ContentRight()
{
	const Theme& t = GetTheme();
	Button(U"\U0001F4C5",
		[this]() { ToggleCalendarVisibility(calendarCtx); },
		id("Select day"),
		w(16), h(16),
		m(2),
		fontSize(8),
		size_none,
		hoverBg(rgba(0, 0, 0, 0), t.overlay0),
		rounded(999)
	);
}

void SableUI::DatePickerComponent::OnUpdatePostLayout(const UIEventContext& ctx)
{
	TextFieldComponent::OnUpdatePostLayout(ctx);

	CalendarHelperPostLayout(calendarCtx, "Select day");
}
