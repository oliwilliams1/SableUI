#include "SableUI.h"

int main()
{
	SableUI::CreateWindow("SableUI", 800, 600);
	SableUI::SetMaxFPS(60);

	while (SableUI::PollEvents())
	{
		SableUI::Draw();
	}

	SableUI::Destroy();
}