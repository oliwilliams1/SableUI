#include "SableUI/SableUI.h"

int main(int argc, char** argv)
{
	SableUI::Window mainWindow("SableUI", 1000, 800);

	mainWindow.OpenUIFile("template.sbml");

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
	el2.bgColour = SableUI::Colour(255, 0, 255);
	el2.padding = 5.0f;
	mainWindow.AddElementToComponent("component 3", el2, SableUI::ElementType::RECT);

	SableUI::ElementInfo el3{};
	el3.name = "element 3";
	el3.wType = SableUI::RectType::FIXED;
	el3.hType = SableUI::RectType::FIXED;
	el3.padding = 5.0f;
	el3.centerX = true;
	SableUI::Element* imageElement = mainWindow.AddElementToComponent("component 3", el3, SableUI::ElementType::IMAGE);
	imageElement->width = 128.0f;
	imageElement->height = 128.0f;
	imageElement->SetImage("img.jpg");

	SableUI::ElementInfo el4{};
	el4.name = "element 4";
	el4.wType = SableUI::RectType::FIXED;
	el4.hType = SableUI::RectType::FIXED;
	el4.padding = 5.0f;
	el4.centerX = true;
	el4.width = 128.0f;
	el4.height = 128.0f;
	SableUI::Element* imageElement1 = mainWindow.AddElementToComponent("component 3", el4, SableUI::ElementType::IMAGE);
	imageElement1->SetImage("bomb.png");

	SableUI::ElementInfo el5{};
	el5.wType = SableUI::RectType::FILL;
	el5.hType = SableUI::RectType::FIXED;
	el5.height = 24.0f;
	SableUI::Element* textElement1 = mainWindow.AddElementToComponent("component 3", el5, SableUI::ElementType::TEXT);
	textElement1->SetText(U"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. | (#♥) | (＜★) | 이브 프시케 カタカナ 漢字 ひらがな พยัญชนะ 汉字 स्वर äöüçñæ 生而平等 ¿ 😔", 14);

	while (mainWindow.PollEvents())
	{
		mainWindow.Draw();
	}

	return 0;
}