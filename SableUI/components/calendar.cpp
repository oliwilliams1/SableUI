#include <SableUI/SableUI.h>
#include <SableUI/components/calendar.h>
#include <SableUI/components/button.h>
#include <SableUI/styles/styles.h>
#include <SableUI/styles/theme.h>
#include <SableUI/utils/utils.h>
#include <ctime>

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

Calendar::Calendar()
{
	auto now = std::time(nullptr);
	auto* tm = std::localtime(&now);

	viewYear.set(tm->tm_year + 1900);
	viewMonth.set(tm->tm_mon);

	selectedYear.set(viewYear.get());
	selectedMonth.set(viewMonth.get());
	selectedDay.set(tm->tm_mday);
}

void Calendar::Layout()
{
	const Theme& t = GetTheme();
	const char* monthNames[] = {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	};

	const int cellSize = 32;

	Div(bg(t.surface0), roundedTop(8), b(2), bb(0), borderColour(t.red))
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
				SableString::Format("%s %d", monthNames[viewMonth.get()], viewYear.get()).bold(),
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

	RenderDays(cellSize);
}

void Calendar::PrevMonth()
{
	int m = viewMonth.get() - 1;
	int y = viewYear.get();

	if (m < 0) { m = 11; y--; }

	viewMonth.set(m);
	viewYear.set(y);
}

void Calendar::NextMonth()
{
	int m = viewMonth.get() + 1;
	int y = viewYear.get();

	if (m > 11) { m = 0; y++; }

	viewMonth.set(m);
	viewYear.set(y);
}

bool Calendar::IsSelectedDay(int y, int m, int d) const
{
	return selectedDay.get() == d &&
		selectedMonth.get() == m &&
		selectedYear.get() == y;
}

void Calendar::RenderDays(int cellSize)
{
	const auto& t = GetTheme();

	int vy = viewYear.get();
	int vm = viewMonth.get();

	int first = FirstWeekday(vy, vm);
	int dim = DaysInMonth(vy, vm);

	int day = 1;

	for (int week = 0; week < 6; week++)
	{
		Div(left_right, b(2), bt(0), borderColour(t.red))
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
					bool selected = IsSelectedDay(vy, vm, thisDay);

					Colour baseColour = selected ? t.primary : rgba(0, 0, 0, 0);
					Colour hoverColour = selected ? t.primary : t.surface1;

					Button(
						SableString::Format("%d", thisDay),
						([this, vy, vm, thisDay]() {
							selectedYear.set(vy);
							selectedMonth.set(vm);
							selectedDay.set(thisDay);
							}),
						w(cellSize),
						h(cellSize),
						centerXY,
						justify_center,
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