#include "SableUI/SableUI.h"

int main()
{
	SableUI::SBCreateWindow("SableUI", 800, 600);

	SableUI::OpenUIFile("template.SableUI");

	SableUI::ElementInfo el2info{};
	el2info.name = "element 2";
	el2info.wType = SableUI::RectType::FILL;
	el2info.hType = SableUI::RectType::FILL;
	el2info.bgColour = SableUI::colour(255, 0, 0);
	el2info.padding = 5.0f;
	SableUI::AddElementToComponent("component 3", el2info);

	SableUI::ElementInfo el3info{};
	el3info.name = "element 3";
	el3info.wType = SableUI::RectType::FILL;
	el3info.hType = SableUI::RectType::FILL;
	el3info.bgColour = SableUI::colour(0, 255, 255);
	el3info.padding = 15.0f;
	SableUI::AddElementToComponent("component 3", el3info);

	SableUI::RecalculateNodes();

	SableUI::PrintNodeTree();

	while (SableUI::PollEvents())
	{
		SableUI::Draw();
	}

	SableUI::Destroy();
}