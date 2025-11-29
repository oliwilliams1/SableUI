#include <utility>
#include <algorithm>
#include <cstdint>
#include <string>

#include <SableUI/SableUI.h>
#include <SableUI/components/debugComponents.h>
#include <SableUI/component.h>
#include <SableUI/console.h>
#include <SableUI/element.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>
#include <SableUI/window.h>
#include <SableUI/renderer.h>

using namespace SableUI;
class ToggleImageView : public BaseComponent
{
public:
	ToggleImageView() : BaseComponent() {};

	void Layout() override
	{
		std::string path = (toggleState) ? "1.webp" : "1.webp";

		Div(w(128) h_fit)
		{
			Div(bg(128, 32, 32) p(4) w_fill
				onClick([=]() { setToggleState(!toggleState); }))
			{
				Text("Click to change image. Loaded: " + path, textColour(0, 0, 0) mb(4));
			}

			Image(path, w(128) h(128));
		}
	}

private:
	useState(toggleState, setToggleState, bool, false);
};

class TestComponent : public BaseComponent
{
public:
	TestComponent(int r) : BaseComponent(SableUI::Colour(r, 32, 32)), v(r) {}

	void Layout() override
	{
		SableUI::LayoutDirection direction = (v == 128) ? SableUI::LayoutDirection::RIGHT_LEFT : SableUI::LayoutDirection::LEFT_RIGHT;

		Div(dir(direction) w_fill h_fill bg(255, 0, 0))
		{
			Div(p(5) bg(isHovered ? rgb(255, 255, 255) : rgb(0, 255, 255)) w(50) h(50)
				onHover([=]() { setIsHovered(true); })
				onHoverExit([=]() { setIsHovered(false); }))
			{
				Rect(w(20) h(20) bg(clicks, 0, 255)
					onClick([=]() { setClicks(clicks + 40); })
					onSecondaryClick([=]() { setClicks(clicks - 40); }));
			}

			Div(m(5) w(100) h(100) bg(255, 255, 0))
			{
				Rect(px(5) py(5) bg(255, 0, 255) w(50) h(50));
			}
			Component(ToggleImageView, w_fit h_fit p(5));
			Component(ToggleImageView, w_fit h_fit p(5));
		}
		Div(w_fill bg(v, 0, 0))
		{
			Rect(w_fill minW(250) maxW(300) h(75) bg(128, 128, 128));
			Rect(m(5) w(60) h(60) bg(255, 128, 0));
		}
	}

private:
	useState(isHovered, setIsHovered, bool, false);
	useState(clicks, setClicks, uint8_t, 0);
	int v;
};

class ImageView : public BaseComponent
{
public:
	ImageView(std::string  path, const int width = 128, const int height = 128)
		: BaseComponent(), m_path(std::move(path)), width(width), height(height) {};

	void Layout() override
	{
		Image(m_path, w(width) h(height) centerXY
			onHover([=]() { setState(false); })
			onHoverExit([=]() { setState(true); }));

		if (state)
		{
			Div(ID("text parent") bg(80, 0, 0) h_fit w_fill p(5))
			{
				TextU32("Automatic truncated text: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.", minW(100) justify_center maxH(20) mb(4));
				TextU32("Non-truncated text: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.", minW(100) justify_center);
			}
		}
		else
		{
			Div(w_fill h_fill bg(128, 128, 255) p(5))
			{
				Rect(w(10) h(10) centerXY bg(0, 255, 0));
			}
			Image(m_path, w(width) h(height) centerXY);
		}
	}

private:
	std::string m_path;
	int width, height;
	useState(state, setState, bool, true);
};

class ConsoleView : public BaseComponent
{
public:
	ConsoleView() : BaseComponent() {};

	void Layout() override
	{
		rootElement->setPadding(4);

		Text("Console", fontSize(24));

		int nLogs = SableUI::Console::m_Logs.size();

		for (int i = std::max(nLogs - 18, 0); i < nLogs; i++)
		{
			const auto& logData = SableUI::Console::m_Logs[i];
			Colour logColour = rgb(255, 255, 255);

			switch (logData.type)
			{
			case LogType::SBUI_LOG: logColour = rgb(0, 255, 0); break;
			case LogType::SBUI_WARNING: logColour = rgb(255, 255, 0); break;
			case LogType::SBUI_ERROR: logColour = rgb(255, 0, 0); break;
			default: logColour = rgb(255, 255, 255); break;
			}

			Text(logData.message, textColour(logColour));
		}
	}

	void OnUpdate(const UIEventContext& ctx) override
	{
		if (nLogs != Console::m_Logs.size())
		{
			setNlogs(Console::m_Logs.size());
		}
	}

private:
	useState(nLogs, setNlogs, int, 0);
};

class MenuBar : public BaseComponent
{
public:
	MenuBar(Window* window) : BaseComponent() { m_window = window; };

	void DrawMenuBarItem(const std::string& text)
	{
		Div(p(2) mb(2) bg(32, 32, 32) w_fit
			onClick([=]()
			{
				UseCustomTargetQueue(queue, m_window, m_window->GetSurface())
				{
					Div(w(128) h(128) bg(32, 32, 32, 255) p(50))
					{
						Text(SableString::Format("Clicked: %s", text.c_str()));
					}
				}
			}))
		{
			Text(text, justify_center h_fit w_fit px(4));
		}
	}

	void Layout() override
	{
		Div(left_right bg(32, 32, 32))
		{
			DrawMenuBarItem("File");
			DrawMenuBarItem("Edit");
			DrawMenuBarItem("View");
			DrawMenuBarItem("...");
		}
	}  

private:
	Window* m_window = nullptr;
	CustomLayoutContext(queue);
};

class Counter : public SableUI::BaseComponent
{
public:
	Counter() : SableUI::BaseComponent() {}

	void Layout() override
	{
		Div(bg(245, 245, 245) p(30) centerXY w_fit h_fit rounded(10))
		{
			Text(SableString::Format("Count: %d", count),
				fontSize(28) mb(20) textColour(20, 20, 20) justify_center);

			Div(left_right p(4) centerX rounded(9))
			{
				Div(bg(90, 160, 255) p(8) mr(5) rounded(5)
					onClick([=]() { setCount(count + 1); }))
				{
					Text("Increment",
						mb(2) textColour(255, 255, 255) fontSize(16) justify_center);
				}

				Div(bg(255, 120, 120) p(8) rounded(5)
					onClick([=]() { setCount(count - 1); }))
				{
					Text("Decrement",
						mb(2) textColour(255, 255, 255) fontSize(16) justify_center);
				}
			}
		}
	}

private:
	useState(count, setCount, int, 0);
};

int main(int argc, char** argv)
{
	SableUI::PreInit(argc, argv);
	//SableUI::SetBackend(SableUI::Backend::Vulkan);
	SableUI::Window* mainWindow = SableUI::Initialise("SableUI Test", 1600, 900);
	SableUI::SetMaxFPS(60);

	VSplitter()
	{
		SableUI::SetNextPanelMaxHeight(20);
		PanelWith(MenuBar, mainWindow);
		HSplitter()
		{
			VSplitter()
			{
				HSplitter()
				{
					PanelWith(TestComponent, 128);
					VSplitter()
					{
						PanelWith(Counter);
						HSplitter()
						{
							PanelWith(TestComponent, 80);
							PanelWith(ImageView, "3.jpg", 128, 128);
						}
					}
				}
				SableUI::SetNextPanelMaxHeight(250);
				PanelWith(ConsoleView);

			}
			Panel();
		}
	}

	SableUI::CreateSecondaryWindow("Debug View", 250, 900);
	VSplitter()
	{
		PanelWith(SableUI::ElementTreeView, mainWindow);
		PanelWith(SableUI::PropertiesView);
	}

	//SableUI::CreateSecondaryWindow("Debug View", 250, 900);
	//VSplitter()
	//{
	//	PanelWith(SableUI::ElementTreeView, mainWindow);
	//	PanelWith(SableUI::PropertiesView);
	//}

	while (SableUI::PollEvents())
	{
		SableUI::Render();
	}

	SableUI::Shutdown();
	return 0;
}