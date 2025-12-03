#include <SableUI/components/tabStack.h>
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/utils.h>
#include <SableUI/events.h>

void SableUI::TabStackPanel::Layout()
{
	Div(w_fill h_fit left_right bg(45, 45, 45))
	{
		for (int i = 0; i < tabs.size(); i++)
		{
			bool isActive = (i == activeTab);
			Div(bg(isActive ? rgb(70, 70, 70) : rgb(50, 50, 50))
				p(4) mr(2) rounded(4)
				onClick([this, i]() { setActiveTab(i); }))
			{
				Text(SableString::Format("Tab-%d", i + 1),
					textColour(isActive ? 255, 255, 255 : 180, 180, 180));
			}
		}
	}

	if (!tabs.empty() && activeTab >= 0 && activeTab < (int)tabs.size())
	{
		Div(w_fill h_fill bg(32, 32, 32))
		{
			tabs[activeTab]->LayoutWrapper();
		}
	}
}

void SableUI::TabStackPanel::OnUpdate(const UIEventContext& ctx)
{
	if (activeTab >= 0 && activeTab < (int)tabs.size())
		tabs[activeTab]->OnUpdate(ctx);
}

void SableUI::TabStackPanel::comp_PropagateEvents(const UIEventContext& ctx)
{
	BaseComponent::comp_PropagateEvents(ctx);

	if (activeTab >= 0 && activeTab < (int)tabs.size())
		tabs[activeTab]->comp_PropagateEvents(ctx);
}

bool SableUI::TabStackPanel::comp_PropagateComponentStateChanges(bool* changed)
{
	bool baseChanged = BaseComponent::comp_PropagateComponentStateChanges(changed);

	if (activeTab >= 0 && activeTab < (int)tabs.size())
	{
		bool childChanged = tabs[activeTab]->comp_PropagateComponentStateChanges(changed);
		return baseChanged || childChanged;
	}

	return baseChanged;
}