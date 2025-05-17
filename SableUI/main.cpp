#include "SableUI/SableUI.h"

int main()
{
	SableUI::SBCreateWindow("SableUI", 800, 600);

	SableUI::OpenUIFile("template.SableUI");

	BaseElement element = BaseElement(SableUI::rect(0.0f, 0.0f, 0.0f, 12.0f, SableUI::RectType::FILL, SableUI::RectType::FIXED),
		SableUI::colour(255, 0, 0));
	SableUI::AddElementToComponent("component 3", std::make_unique<BaseElement>(element));

	SableUI::PrintNodeTree();

	while (SableUI::PollEvents())
	{
		SableUI::Draw();
	}

	SableUI::Destroy();
}