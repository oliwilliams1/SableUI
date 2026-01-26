#pragma once
#include <SableUI/core/component.h>

namespace SableUI
{
	class Calendar : public BaseComponent
	{
	public:
		Calendar();
		void Layout() override;

	private:
		State<int> viewYear{ this, 0 };
		State<int> viewMonth{ this, 0 };
		State<int> selectedYear{ this, 0 };
		State<int> selectedMonth{ this, 0 };
		State<int> selectedDay{ this, -1 };

		void PrevMonth();
		void NextMonth();
		bool IsSelectedDay(int y, int m, int d) const;
		void RenderDays(int cellW);
	};
}