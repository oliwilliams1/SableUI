#pragma once
#include <SableUI/core/component.h>

namespace SableUI
{
	struct CalendarContext
	{
		int viewYear = -1;
		int viewMonth = -1;
		int selectedYear = -1;
		int selectedMonth = -1;
		int selectedDay = -1;

		const bool operator==(const CalendarContext& other) const
		{
			return (viewYear == other.viewYear
				&& viewMonth == other.viewMonth
				&& selectedYear == other.selectedYear
				&& selectedMonth == other.selectedMonth
				&& selectedDay == other.selectedDay);
		}
	};

	class Calendar : public BaseComponent
	{
	public:
		void Init(State<CalendarContext>& calendarContext);
		void Layout() override;
		void SetSelectedDate(int year, int month, int day);

	private:
		State<CalendarContext>* m_data = nullptr;

		void PrevMonth();
		void NextMonth();
		bool IsSelectedDay(int y, int m, int d, const CalendarContext& ctx) const;
		void RenderDays(int cellW, const CalendarContext& ctx);
	};
}