#include <SableUI/components/date_picker.h>
#include <SableUI/styles/styles.h>
#include <SableUI/SableUI.h>

using namespace SableUI::Style;

void SableUI::DatePicker::ContentRight()
{
	const Theme& t = GetTheme();
	Button(U"\U0001F4C5", // unicode calendar emoji
		[this]() { calendarOpen.set(!calendarOpen.get()); },
		w(18), h(18), mr(4),
		hoverBg(rgba(0, 0, 0, 0), t.surface2), 
		rounded(999), size_none,
		id("date picker"));
}

void SableUI::DatePicker::TextFieldOnUpdatePostLayout(const UIEventContext& ctx)
{
	if (calendarOpen.get() && !IsFloatingPanelActive("Calendar"))
	{
		Element* ref = GetElementById("date picker");
		if (!ref) return;

		Rect bottomRight = ref->rect;
		bottomRight.y += bottomRight.h;

		FloatingPanelScoped(ref, Calendar, "Calendar", Rect(bottomRight.x, bottomRight.y, 400, 400))
		{
			ref->Init(calendarCtx);
		}
	}
	else if (!calendarOpen.get() && IsFloatingPanelActive("Calendar"))
	{
		QueueDestroyFloatingPanel("Calendar");
	}
}