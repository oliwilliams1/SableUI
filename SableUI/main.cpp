#include "SableUI/SableUI.h"

int main()
{
	SableUI::SBCreateWindow("SableUI", 800, 600);

	SableUI::OpenUIFile("template.SableUI");

	BaseElement element = BaseElement(0, 20);
	element.wType = SableUI::RectType::FILL;
	element.hType = SableUI::RectType::FIXED;
	SableUI::AddElementToComponent("component 3", std::make_unique<BaseElement>(element));

	BaseElement element2 = BaseElement(0, 20);
	element2.wType = SableUI::RectType::FILL;
	element2.hType = SableUI::RectType::FIXED;
	element2.bgColour = SableUI::colour(255, 0, 0);
	element2.padding = 5.0f;
	SableUI::AddElementToComponent("component 3", std::make_unique<BaseElement>(element2));

	SableUI::RecalculateNodes();

	SableUI::PrintNodeTree();

	while (SableUI::PollEvents())
	{
		SableUI::Draw();
	}

	SableUI::Destroy();
}