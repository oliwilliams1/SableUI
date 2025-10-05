#include "SableUI/SableUI.h"

class ToggleImageView : public SableUI::BaseComponent
{
public:
	ToggleImageView() : SableUI::BaseComponent() {};

	void Layout() override
	{
		std::string path = (toggleState) ? "bomb.webp" : "junior.jpg";

		Div(w(128) h_fit)
		{
			Div(bg(128, 32, 32) p(2)
				onClick([&]() { setToggleState(!toggleState); }))
			{
				Text("Click to change image. Loaded: " + path);
			}

			Image(path, w(128) h(128));
		}
	}

private:
	useState(toggleState, setToggleState, bool, false);
};

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

			Component(ToggleImageView, w_fit h_fit p(5) bg(rgb(0, 0, 0)));
			Component(ToggleImageView, w_fit h_fit p(5) bg(rgb(0, 0, 0)));
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
		std::u32string pathU32 = std::u32string(m_path.begin(), m_path.end());

		Image(m_path, w(width) h(height) centerXY
			onHover([&]() { setText(U"unicode test ⟡ ↀ 안녕하세요, 제 이름은 오리 입니다. 저 는 열일곱 살 입니다. 저는 뉴젠스 좋압니다."); })
			onHoverExit([&]() { setText(U"lorem ipsum"); }));

		Div(id("text parent") bg(80, 0, 0) h_fit p(5))
		{
			TextU32(text, minW(100));
		}
	}

private:
	std::string m_path;
	int width, height;
	useState(text, setText, SableString, U"lorem ipsum");
};

int main(int argc, char** argv)
{
	SableUI::PreInit(argc, argv);
	//SableUI::SetBackend(SableUI::Backend::Vulkan);
	SableUI::Initialise("SableUI Test", 1600, 900);

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
			PanelWith(ToggleImageView);

		}
		VSplitter()
		{
			Panel();
			Panel();
		}
	}

	//SableUI::CreateSecondaryWindow();
	//PanelWith(ImageView, "dirtywork.jpg", 160, 160);

	while (SableUI::PollEvents())
	{
		SableUI::Render();
	}

	SableUI::Shutdown();

	return 0;
}