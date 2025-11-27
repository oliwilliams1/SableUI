#pragma once
#include <SableUI/component.h>

namespace SableUI
{
	class DropdownMenu : public BaseComponent
	{
	public:
		DropdownMenu(bool visible) : BaseComponent(), visible(visible) {};

		void Layout() override;

	private:
		bool visible = false;
	};
}