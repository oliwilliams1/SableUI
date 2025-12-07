#include <SableUI/SableUI.h>
#include <SableUI/component.h>
#include <SableUI/componentRegistry.h>
#include <SableUI/components/menuBar.h>
#include <SableUI/components/debugComponents.h>
#include <SableUI/components/tabStack.h>
#include <SableUI/window.h>
#include <SableUI/console.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using namespace SableUI;

class ToggleImageView : public BaseComponent
{
public:
	void Layout() override
	{
		std::string path = (toggleState) ? "1.webp" : "1.webp";

		Div(w(128) h_fit)
		{
			Div(bg(128, 32, 32) p(4) w_fill
				onClick([this]() { setToggleState(!toggleState); }))
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
	void Layout() override
	{
		Div(left_right w_fill h_fill bg(255, 0, 0))
		{
			Div(p(5) bg(isHovered ? rgb(255, 255, 255) : rgb(0, 255, 255)) w(50) h(50)
				onHover([this]() { setIsHovered(true); })
				onHoverExit([this]() { setIsHovered(false); }))
			{
				Rect(w(20) h(20) bg(clicks, 0, 255)
					onClick([this]() { setClicks(clicks + 40); })
					onSecondaryClick([this]() { setClicks(clicks - 40); }));
			}

			Div(m(5) w(100) h(100) bg(255, 255, 0))
			{
				Rect(px(5) py(5) bg(255, 0, 255) w(50) h(50));
			}
			Component("ToggleImageView", w_fit h_fit p(5));
			Component("ToggleImageView", w_fit h_fit p(5));
		}
		Div(w_fill bg(180, 0, 0))
		{
			Rect(w_fill minW(250) maxW(300) h(75) bg(128, 128, 128));
			Rect(m(5) w(60) h(60) bg(255, 128, 0));
		}
	}

private:
	useState(isHovered, setIsHovered, bool, false);
	useState(clicks, setClicks, uint8_t, 0);
};

class ImageView : public BaseComponent
{
public:
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

	void InitData(const std::string& path, int w, int h)
	{ 
		m_path = path; width = w; height = h;
		needsRerender = true;
	}

private:
	std::string m_path;
	int width, height;
	useState(state, setState, bool, true);
};

class ConsoleView : public BaseComponent
{
public:
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

class Counter : public SableUI::BaseComponent
{
public:
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

class RefTest : public SableUI::BaseComponent
{
public:
	void Layout() override
	{
		Div(bg(50, 50, 50) p(20) w_fill rounded(10))
		{
			Div(bg(70, 70, 120) p(15) rounded(5) mb(20) w_fill centerX)
			{
				Text(SableString::Format("State counter: %d", stateCounter),
					fontSize(20) mb(10));

				Div(bg(100, 100, 200) p(8) rounded(5) w_fill
					onClick([this]() {
						setStateCounter(stateCounter + 1);
					}))
				{
					Text("Increment useState");
				}
			}

			Div(bg(120, 70, 70) p(15) rounded(5) w_fill centerX)
			{
				Text(SableString::Format("Ref counter: %d", refCounter),
					fontSize(20) mb(10));

				Div(bg(200, 100, 100) p(8) rounded(5) w_fill
					onClick([this]() {
						refCounter++;
					}))
				{
					Text("Increment useRef");
				}
			}
		}
	}

private:
	useState(stateCounter, setStateCounter, int, 0);
	useRef(refCounter, int, 0);
};

class RefTestParent : public SableUI::BaseComponent
{
public:
	void Layout() override
	{
		Div(p(30) bg(40, 40, 40) w_fill)
		{
			Div(bg(32, 80, 128) p(10) mb(20) w_fill rounded(5)
				onClick([this]() {
					setShow(!show);
				}))
			{
				Text("Toggle RefTest", justify_center);
			}

			Div(bg(32, 80, 128) p(10) mb(20) w_fill rounded(5)
				onClick([this]() {
					needsRerender = true;
				}))
			{
				Text("Rerender Parent", justify_center);
			}

			if (show)
			{
				Component("RefTest", w_fill bg(50, 50, 50) rounded(10));
			}
		}
	}

private:
	useState(show, setShow, bool, true);
};


int main(int argc, char** argv)
{
	PreInit(argc, argv);

	RegisterComponent<Counter>("Counter");
	RegisterComponent<RefTestParent>("RefTestParent");
	RegisterComponent<RefTest>("RefTest");
	RegisterComponent<TestComponent>("TestComponent");
	RegisterComponent<ToggleImageView>("ToggleImageView");
	RegisterComponent<ImageView>("ImageView");
	RegisterComponent<ConsoleView>("ConsoleView");
	
	Window* mainWindow = Initialise("SableUI", 1600, 900);
	SetMaxFPS(200);

	VSplitter()
	{
		SetNextPanelMaxHeight(20);
		PanelGainRef("Menu Bar", MenuBar, menuBarRef);
		menuBarRef->SetWindow(mainWindow);
		HSplitter()
		{
			VSplitter()
			{
				HSplitter()
				{
					SetNextPanelMinBounds({ 500, 400 });
					Panel("TestComponent");
					HSplitter()
					{
						SetNextPanelMinBounds({ 500, 400 });
						PanelGainRef("TabStack", TabStack, tabStackRef);
						tabStackRef->AddTab("Counter");
						tabStackRef->AddTab("TestComponent");
						tabStackRef->AddTab("ImageView");
						tabStackRef->AddTab("RefTestParent");
					}
				}
				SetNextPanelMaxHeight(250);
				SetNextPanelMinBounds({ 50, 250 });
				Panel("ConsoleView");

			}
			SetNextPanelMinBounds({ 300, 20 });
			Panel("RefTestParent");
		}
	}

	SableUI::CreateSecondaryWindow("Debug View", 250, 900);
	VSplitter()
	{
		PanelGainRef("ElementTreeView", ElementTreeView, elementTreeViewRef);
		elementTreeViewRef->SetWindow(mainWindow);
		Panel("PropertiesView");
	}

	while (SableUI::PollEvents())
		SableUI::Render();

	SableUI::Shutdown();
	return 0;
}