#include "SableUI/SableUI.h"

int main(int argc, char** argv)
{
	SableUI::Window mainWindow("SableUI", 800, 600);

	mainWindow.OpenUIFile("template.SableUI");

	SableUI::ElementInfo el1{};
	el1.name = "element 1";
	el1.wType = SableUI::RectType::FILL;
	el1.hType = SableUI::RectType::FIXED;
	el1.height = 20.0f;
	el1.bgColour = SableUI::Colour(255, 0, 0);
	mainWindow.AddElementToComponent("component 3", el1, SableUI::ElementType::RECT);

	SableUI::ElementInfo el2{};
	el2.name = "element 2";
	el2.wType = SableUI::RectType::FILL;
	el2.hType = SableUI::RectType::FILL;
	el2.bgColour = SableUI::Colour(255, 0, 0);
	el2.padding = 5.0f;
	mainWindow.AddElementToComponent("component 3", el2, SableUI::ElementType::RECT);

	SableUI::ElementInfo el3{};
	el3.name = "element 3";
	el3.wType = SableUI::RectType::FILL;
	el3.hType = SableUI::RectType::FILL;
	el3.bgColour = SableUI::Colour(0, 255, 255);
	el3.padding = 15.0f;
	mainWindow.AddElementToComponent("component 3", el3, SableUI::ElementType::RECT);

	SableUI::ElementInfo el4{};
	el4.name = "element 4";
	el4.wType = SableUI::RectType::FIXED;
	el4.hType = SableUI::RectType::FIXED;
	el4.bgColour = SableUI::Colour(255, 0, 255);
	el4.height = 20.0f;
	el4.width = 20.0f;
	el4.padding = 5.0f;
	el4.centerX = true;
	mainWindow.AddElementToComponent("component 3", el4, SableUI::ElementType::RECT);

	while (mainWindow.PollEvents())
	{
		mainWindow.Draw();
	}
}