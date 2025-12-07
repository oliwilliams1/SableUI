#include <SableUI/components/tabStack.h>
#include <SableUI/SableUI.h>
#include <SableUI/component.h>
#include <vector>
#include <string>
#include <SableUI/console.h>

using namespace SableUI;

void TabStack::Layout()
{
	Div(w_fill h_fit left_right bg(45, 45, 45))
	{
		for (int i = 0; i < tabs.size(); i++)
		{
			std::string& tabLabel = tabs[i].label;
			bool isActive = (i == activeTab);
			Div(bg(isActive ? rgb(70, 70, 70) : rgb(50, 50, 50))
				p(4) mr(4)
				onClick([this, i]() { setActiveTab(i); }))
			{
				Text(tabLabel);
			}
		}
	}
	Div(w_fill h_fill bg(32, 32, 32))
	{
		if (tabs.empty())
		{
			Text("No tabs", centerY w_fill justify_center);
		}
		else if (activeTab < tabs.size())
		{
			Component(tabs[activeTab].component.c_str(), w_fill h_fill bg(32, 32, 32));
		}
		else
		{
			Text("Invalid tab index", centerY w_fill justify_center);
		}
	}
}

static bool TabExists(const std::vector<_TabDef>& tabs, const std::string& tabName)
{
	for (int i = 0; i < tabs.size(); i++)
	{
		if (tabs[i].label == tabName)
		{
			return true;
		}
	}
	return false;
}

void TabStack::AddTab(const std::string& componentName, const std::string& label)
{
	if (TabExists(tabs, label))
	{
		SableUI_Warn("Tab with label '%s' already exists, will be skipped", label.c_str());
		return;
	}

	tabs.push_back({ componentName, label });
	needsRerender = true;
}

void TabStack::AddTab(const std::string& componentName)
{
	if (TabExists(tabs, componentName))
	{
		SableUI_Warn("Tab with label '%s' already exists, will be skipped", componentName.c_str());
		return;
	}

	tabs.push_back({ componentName, componentName });
	needsRerender = true;
}