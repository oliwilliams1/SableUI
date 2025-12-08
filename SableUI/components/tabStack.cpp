#include <SableUI/components/tabStack.h>
#include <SableUI/SableUI.h>
#include <SableUI/component.h>
#include <vector>
#include <string>
#include <map>
#include <SableUI/console.h>

using namespace SableUI;

void _TabStackDef::Layout()
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
				Text(tabLabel, wrapText(false));
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
			BaseComponent* ref = AddComponent(tabs[activeTab].component.c_str());

			if (ref != nullptr && tabs[activeTab].initialiser)
			{
				tabs[activeTab].initialiser(ref);
			}

			ref->BackendInitialiseChild(tabs[activeTab].component.c_str(), this, style(w_fill h_fill bg(32, 32, 32)));
		}
		else
		{
			Text("Invalid tab index", centerY w_fill justify_center);
		}
	}
}

bool _TabStackDef::TabExists(const std::string& tabName)
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

void _TabStackDef::AddTab(const std::string& componentName, const std::string& label)
{
	if (TabExists(label))
	{
		SableUI_Warn("Tab with label '%s' already exists, will be skipped", label.c_str());
		return;
	}
	tabs.push_back({ componentName, label, nullptr });
	needsRerender = true;
}

void _TabStackDef::AddTab(const std::string& componentName)
{
	if (TabExists(componentName))
	{
		SableUI_Warn("Tab with label '%s' already exists, will be skipped", componentName.c_str());
		return;
	}
	tabs.push_back({ componentName, componentName, nullptr });
	needsRerender = true;
}