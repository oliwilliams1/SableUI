#include <SableUI/SableUI.h>
#include <SableUI/components/calendar.h>
#include <SableUI/components/button.h>
#include <SableUI/styles/styles.h>
#include <SableUI/styles/theme.h>
#include <SableUI/utils/utils.h>
#include <ctime>
#include <SableUI/core/component.h>

static int DaysInMonth(int year, int month)
{
	static const int days[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	if (month == 1)
	{
		bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
		return leap ? 29 : 28;
	}
	return days[month];
}

static int FirstWeekday(int year, int month)
{
	std::tm t{};
	t.tm_year = year - 1900;
	t.tm_mon = month;
	t.tm_mday = 1;
	std::mktime(&t);
	return t.tm_wday;
}

using namespace SableUI;
using namespace SableUI::Style;

void Calendar::Init(State<CalendarContext>& calendarContext)
{
	m_data = &calendarContext;
	const CalendarContext& dataRef = m_data->get();
	
	bool unitialised =
		dataRef.viewYear == -1
		|| dataRef.viewMonth == -1
		|| dataRef.selectedDay == -1
		|| dataRef.selectedMonth == -1
		|| dataRef.selectedYear == -1;

	if (unitialised)
		if (InitCalendarToToday(*m_data))
			MarkDirty();
}

void Calendar::Layout()
{
	const Theme& t = GetTheme();
	const char* monthNames[] = {
		"NULL",
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	};

	const int cellSize = 28; // VARY BASED ON COMPONENT SIZE

	if (!m_data)
	{
		Text("Calendar component has a null context, call Init() on the Calendar component passing a valid State<CalendarContext> in the parent component",
			textColour(t.error));
		return;
	}

	const CalendarContext& ctx = m_data->get();

	Div(b(1), borderColour(t.overlay1), rounded(8), bg(t.mantle))
	{
		Div(bg(t.surface0), roundedTop(8), bg(t.surface0), bb(1), borderColour(t.overlay0), px(4))
		{
			Div(left_right, mb(3), w_fill, p(4))
			{
				Button(
					"<",
					[this]() {
						PrevMonth();
					},
					w(32), h(32),
					fontSize(14),
					hoverBg(rgba(0, 0, 0, 0), t.surface1),
					rounded(999),
					textColour(t.subtext0)
				);
				Text(
					SableString::Format("%s %d", monthNames[ctx.viewMonth + 1], ctx.viewYear).bold(),
					centerXY,
					fontSize(14),
					justify_center,
					w_fill,
					textColour(t.subtext1)
				);

				Button(
					">",
					[this]() {
						NextMonth();
					},
					w(32), h(32),
					fontSize(14),
					hoverBg(rgba(0, 0, 0, 0), t.surface1),
					rounded(999),
					textColour(t.subtext0)
				);
			}

			Div(left_right, mb(2), py(8))
			{
				static const char* days[] = { "S", "M", "T", "W", "T", "F", "S" };
				for (auto d : days)
					Text(d, w(cellSize), centerXY, justify_center, fontSize(12), textColour(t.subtext0));
			}
		}

		RenderDays(cellSize, ctx);
	}
}

void Calendar::PrevMonth()
{
	const CalendarContext& ctx = m_data->get();

	int m = ctx.viewMonth - 1;
	int y = ctx.viewYear;

	if (m < 0) { m = 11; y--; }

	CalendarContext dataCopy = { m_data->get() };
	dataCopy.viewMonth = m;
	dataCopy.viewYear = y;

	if (dataCopy != m_data->get())
	{
		m_data->set(dataCopy);
		MarkDirty();
	}
}

void Calendar::NextMonth()
{
	const CalendarContext& ctx = m_data->get();

	int m = ctx.viewMonth + 1;
	int y = ctx.viewYear;

	if (m > 11) { m = 0; y++; }

	CalendarContext dataCopy = { m_data->get() };
	dataCopy.viewMonth = m;
	dataCopy.viewYear = y;

	if (dataCopy != m_data->get())
	{
		m_data->set(dataCopy);
		MarkDirty();
	}
}

bool Calendar::IsSelectedDay(int y, int m, int d, const CalendarContext& ctx) const
{

	return ctx.selectedDay == d &&
		ctx.selectedMonth == m &&
		ctx.selectedYear == y;
}

bool SableUI::InitCalendarToDate(State<CalendarContext>& calendarContextState, int year, int month, int day)
{
	CalendarContext dataCopy = calendarContextState.get();
	dataCopy.selectedYear = year;
	dataCopy.selectedMonth = month;
	dataCopy.selectedDay = day;

	if (dataCopy != calendarContextState.get())
	{
		calendarContextState.set(dataCopy);
		return true;
	}

	return false;
}

bool SableUI::InitCalendarToToday(State<CalendarContext>& calendarContextState)
{
	CalendarContext dataCopy = calendarContextState.get();

	auto now = std::time(nullptr);
	auto* tm = std::localtime(&now);

	dataCopy.viewYear = tm->tm_year + 1900;
	dataCopy.viewMonth = tm->tm_mon;

	dataCopy.selectedYear = dataCopy.viewYear;
	dataCopy.selectedMonth = dataCopy.viewMonth;
	dataCopy.selectedDay = tm->tm_mday;

	if (dataCopy != calendarContextState.get())
	{
		calendarContextState.set(dataCopy);
		return true;
	}

	return false;
}

void Calendar::SetSelectedDate(int y, int m, int d)
{
	if (InitCalendarToDate(*m_data, y, m, d))
		MarkDirty();
}

void Calendar::RenderDays(int cellSize, const CalendarContext& ctx)
{
	const auto& t = GetTheme();

	int vy = ctx.viewYear;
	int vm = ctx.viewMonth;

	int first = FirstWeekday(vy, vm);
	int dim = DaysInMonth(vy, vm);

	int day = 1;

	Div(p(4))
	{
		for (int week = 0; week < 6; week++)
		{
			Div(left_right)
			{
				for (int dow = 0; dow < 7; dow++)
				{
					int cellIndex = week * 7 + dow;

					if (cellIndex < first || day > dim)
					{
						RectElement(w(cellSize), h(cellSize));
					}
					else
					{
						int thisDay = day;
						bool selected = IsSelectedDay(vy, vm, thisDay, ctx);

						Colour baseColour = selected ? t.primary : rgba(0, 0, 0, 0);
						Colour hoverColour = selected ? t.primary : t.surface1;

						Button(
							SableString::Format("%d", thisDay),
							([this, vy, vm, thisDay]() { SetSelectedDate(vy, vm, thisDay); }),
							w(cellSize), h(cellSize),
							centerXY, justify_center,
							hoverBg(baseColour, hoverColour),
							rounded(999),
							fontSize(12),
							size_none
						);

						day++;
					}
				}
			}
		}
	}
}

void SableUI::ToggleCalendarVisibility(State<CalendarContext>& calendarContextState)
{
	CalendarContext dataCopy = { calendarContextState.get() };
	dataCopy.open = !dataCopy.open;
	calendarContextState.set(dataCopy);
}

void SableUI::CalendarHelperPostLayout(State<CalendarContext>& calendarContextState, const std::string& id)
{
	//if (calendarContextState.get().open && !IsFloatingPanelActive(id))
	//{
	//	BaseComponent* owner = calendarContextState._owner();
	//	Element* ref = owner->GetElementById(id);
	//	if (!ref) return;

	//	Rect bottomRight = ref->rect;
	//	bottomRight.y += bottomRight.h;

	//	FloatingPanelScoped(ref, Calendar, id, Rect(bottomRight.x, bottomRight.y, 206, 253), bg(0, 0, 0, 0))
	//	{
	//		ref->Init(calendarContextState);
	//	}
	//}
	//else if (!calendarContextState.get().open && IsFloatingPanelActive(id))
	//{
	//	QueueDestroyFloatingPanel(id);
	//}
}