#include "SableUI/SableUI.h"

int main()
{
	SableUI::SBCreateWindow("SableUI", 800, 600);

	SableUI::OpenUIFile("template.SableUI");

	SableUI::BaseElement element = SableUI::BaseElement(0, 20);
	element.wType = SableUI::RectType::FILL;
	element.hType = SableUI::RectType::FIXED;
	SableUI::AddElementToComponent("component 3", std::make_unique<SableUI::BaseElement>(element));

	SableUI::BaseElement element2 = SableUI::BaseElement(0, 20);
	element2.wType = SableUI::RectType::FILL;
	element2.hType = SableUI::RectType::FIXED;
	element2.bgColour = SableUI::colour(255, 0, 0);
	element2.padding = 5.0f;
	SableUI::AddElementToComponent("component 3", std::make_unique<SableUI::BaseElement>(element2));

	SableUI::RecalculateNodes();

	SableUI::PrintNodeTree();

	while (SableUI::PollEvents())
	{
		SableUI::Draw();
	}

	SableUI::Destroy();
}