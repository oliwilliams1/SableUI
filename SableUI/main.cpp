#include "SableUI/SableUI.h"

int main(int argc, char** argv)
{
	SableUI::Window mainWindow(argc, argv, "SableUI", 1000, 1000);

	mainWindow.OpenUIFile("template.SableUI");

	SableUI::ElementInfo el1{};
	el1.name = "element 1";
	el1.wType = SableUI::RectType::FILL;
	el1.hType = SableUI::RectType::FIXED;
	el1.height = 20.0f;
	el1.bgColour = SableUI::colour(255, 0, 0);
	mainWindow.AddElementToComponent("component 3", el1);

	SableUI::ElementInfo el2{};
	el2.name = "element 2";
	el2.wType = SableUI::RectType::FILL;
	el2.hType = SableUI::RectType::FILL;
	el2.bgColour = SableUI::colour(255, 0, 0);
	el2.padding = 5.0f;
	mainWindow.AddElementToComponent("component 3", el2);

	SableUI::ElementInfo el3{};
	el3.name = "element 3";
	el3.wType = SableUI::RectType::FILL;
	el3.hType = SableUI::RectType::FILL;
	el3.bgColour = SableUI::colour(0, 255, 255);
	el3.padding = 15.0f;
	mainWindow.AddElementToComponent("component 3", el3);

	SableUI::ElementInfo el4{};
	el4.name = "element 4";
	el4.wType = SableUI::RectType::FIXED;
	el4.hType = SableUI::RectType::FIXED;
	el4.bgColour = SableUI::colour(255, 0, 255);
	el4.height = 20.0f;
	el4.width = 20.0f;
	el4.padding = 5.0f;
	el4.centerX = true;
	mainWindow.AddElementToComponent("component 3", el4);

	SableUI::ElementInfo el5{};
	el5.name = "element 5";
	el5.wType = SableUI::RectType::FILL;
	el5.hType = SableUI::RectType::FILL;
	el5.bgColour = SableUI::colour(255, 0, 0);
	el5.padding = 5.0f;
	el5.centerY = true;
	mainWindow.AddElementToComponent("component 9", el5);

	SableUI::ElementInfo el6{};
	el6.name = "element 6";
	el6.wType = SableUI::RectType::FILL;
	el6.hType = SableUI::RectType::FILL;
	el6.bgColour = SableUI::colour(0, 255, 0);
	el6.padding = 5.0f;
	mainWindow.AddElementToElement("element 5", el6);

	for (int i = 0; i < 50; i++)
	{
		SableUI::ElementInfo eltemp{};
		eltemp.name = "element " + std::to_string(i + 7);
		eltemp.wType = SableUI::RectType::FILL;
		eltemp.hType = SableUI::RectType::FILL;
		eltemp.padding = 2.0f;
		mainWindow.AddElementToElement("element 6", eltemp);
	}

	while (mainWindow.PollEvents())
	{
		mainWindow.Draw();
	}
}