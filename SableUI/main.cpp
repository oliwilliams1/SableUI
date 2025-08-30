#include "SableUI/SableUI.h"

class TestComponent : public SableUI::BaseComponent
{
public:
	TestComponent(int r) : SableUI::BaseComponent(SableUI::Colour(r, 32, 32)), v(r) {}

	void Layout() override
	{
		SableUI::LayoutDirection direction = (v == 128) ? SableUI::LayoutDirection::RIGHT_LEFT : SableUI::LayoutDirection::LEFT_RIGHT;

		Div(dir(direction) bg(255, 0, 0))
		{
			Div(id("child") w(50) h(50))
			{
				Div(p(5) bg(isHovered ? rgb(255, 255, 255) : rgb(0, 255, 255)) w(50) h(50)
					onHover([&]() { setIsHovered(true); })
					onHoverExit([&]() { setIsHovered(false); }))
				{
					Rect(w(20) h(20) bg(clicks, 0, 255)
						onClick([&]() { setClicks(clicks + 40); })
						onSecondaryClick([&]() { setClicks(clicks - 40); }));
				}
			}

			Div(m(5) w(100) h(100) bg(255, 255, 0))
			{
				Rect(px(5) py(5) bg(255, 0, 255) w(50) h(50));
			}

		}
		Rect(w_fill minW(250) maxW(300) h(75) bg(128, 128, 128));
		Rect(m(5) w(60) h(60) bg(255, 128, 0));

	}

private:
	useState(isHovered, setIsHovered, bool, false);
	useState(clicks, setClicks, uint8_t, 0);
	int v;
};

class ImageView : public SableUI::BaseComponent
{
public:
	ImageView(const std::string& path, int width = 128, int height = 128)
		: SableUI::BaseComponent(), width(width), height(height), m_path(path) {};

	void Layout() override
	{
		UpdateStyle(rootElement, bg(30, 30, 30));

		std::u32string pathU32 = std::u32string(m_path.begin(), m_path.end());

		Image(m_path, w(width) h(height) centerXY
			onHover([&]() { setText(U"unicode test ⟡ ↀ 첫 눈에 반한다는 그런 설정"); })
			onHoverExit([&]() { setText(U"lorem ipsum"); }));

		Div(id("text parent") bg(80, 0, 0) h_fit p(5))
		{
			TextU32(text);
		}
	}

private:
	std::string m_path;
	int width, height;
	// create own string type to fix?
	useState(text, setText, SableString, U"lorem ipsum");
};

class HoverImageView : public SableUI::BaseComponent
{
public:
	HoverImageView() : SableUI::BaseComponent() {};

	void Layout() override
	{
		std::string path = (isHovered) ? "bomb.webp" : "junior.jpg";

		Div(w(128) h(160))
		{
			Div(bg(128, 32, 32)
				onHover([&]() { setIsHovered(true); })
				onHoverExit([&]() { setIsHovered(false); }))
			{
				Text("Hover to change image, loaded: " + path);
			}

			Image(path, w(128) h(128));
		}
	}

private:
	useState(isHovered, setIsHovered, bool, false);
};

int main(int argc, char** argv)
{
	using namespace SableUI;

	Window mainWindow("SableUI Layout Test", 1600, 1000);
	SetContext(&mainWindow);

	HSplitter()
	{
		VSplitter()
		{
			HSplitter()
			{
				PanelWith(TestComponent, 128);
				VSplitter()
				{
					Panel();
					HSplitter()
					{
						PanelWith(TestComponent, 80);
						PanelWith(ImageView, "dirtywork.jpg", 128, 128);
					}
				}
			}
			PanelWith(HoverImageView);

		}
		VSplitter()
		{
			Panel();
			Panel();
		}
	}

	while (mainWindow.PollEvents())
	{
		mainWindow.Draw();
	}

	return 0;
}