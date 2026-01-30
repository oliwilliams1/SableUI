#pragma once
#include <SableUI/core/component.h>
#include <functional>

namespace SableUI
{
	struct CalendarContext
	{
		int viewYear = -1;
		int viewMonth = -1;
		int selectedYear = -1;
		int selectedMonth = -1;
		int selectedDay = -1;
		bool open = false;

		const bool operator==(const CalendarContext& other) const
		{
			return (viewYear == other.viewYear
				&& viewMonth == other.viewMonth
				&& selectedYear == other.selectedYear
				&& selectedMonth == other.selectedMonth
				&& selectedDay == other.selectedDay
				&& open == other.open);
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

	bool InitCalendarToDate(State<CalendarContext>& calendarContextState, int year, int month, int day);
	bool InitCalendarToToday(State<CalendarContext>& calendarContextState);

	void ToggleCalendarVisibility(State<CalendarContext>& calendarContextState);
	
	/**
	* \brief Called in BaseComponent::OnUpdatePostLayout() that provides the state for calendar components
	* \param calendarContextState the local state for the calendar component
	* \param id the id of the element you want the calendar to appear on top of
	**/
	void CalendarHelperPostLayout(State<CalendarContext>& calendarContextState, const std::string& id);
}
