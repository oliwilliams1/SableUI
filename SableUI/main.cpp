#include "SableUI/SableUI.h"

int main()
{
	SableUI::SBCreateWindow("SableUI", 800, 600);

	SableUI::OpenUIFile("template.SableUI");

	SableUI::BaseElement element = SableUI::BaseElement("element 1", 0, 20);
	element.wType = SableUI::RectType::FILL;
	element.hType = SableUI::RectType::FIXED;
	SableUI::AddElementToComponent("component 3", std::make_unique<SableUI::BaseElement>(element));

	SableUI::BaseElement element2 = SableUI::BaseElement("element 2", 0, 20);
	element2.wType = SableUI::RectType::FILL;
	element2.hType = SableUI::RectType::FILL;
	element2.bgColour = SableUI::colour(255, 0, 0);
	element2.padding = 5.0f;
	SableUI::AddElementToComponent("component 3", std::make_unique<SableUI::BaseElement>(element2));

	SableUI::BaseElement element3 = SableUI::BaseElement("element 3", 0, 20);
	element3.wType = SableUI::RectType::FILL;
	element3.hType = SableUI::RectType::FILL;
	element3.bgColour = SableUI::colour(0, 255, 255);
	element3.padding = 15.0f;
	SableUI::AddElementToComponent("component 3", std::make_unique<SableUI::BaseElement>(element3));

	SableUI::RecalculateNodes();

	SableUI::PrintNodeTree();

	while (SableUI::PollEvents())
	{
		SableUI::Draw();
	}

	SableUI::Destroy();
}