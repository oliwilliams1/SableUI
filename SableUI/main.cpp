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

class ToggleImageView : public SableUI::BaseComponent
{
public:
	ToggleImageView() : SableUI::BaseComponent() {};

	void Layout() override
	{
		std::string path = (toggleState) ? "1.jpg" : "2.jpg";

		Div(w(128) h_fit)
		{
			Div(bg(128, 32, 32) p(2)
				onClick([=]() { setToggleState(!toggleState); }))
			{
				Text("Click to change image. Loaded: " + path, textColour(0, 0, 0));
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
	ImageView(std::string  path, const int width = 128, const int height = 128)
		: SableUI::BaseComponent(), m_path(std::move(path)), width(width), height(height) {};

	void Layout() override
	{
		Image(m_path, w(width) h(height) centerXY
			onHover([=]() { setState(false); })
			onHoverExit([=]() { setState(true); }));

		if (state)
		{
			Div(ID("text parent") bg(80, 0, 0) h_fit p(5))
			{
				TextU32("lorem ipsum", minW(100) justify_center maxH(20));
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

class ConsoleView : public SableUI::BaseComponent
{
public:
	ConsoleView() : SableUI::BaseComponent() {};

	void Layout() override
	{
		rootElement->setPadding(4);

		Text("Console", fontSize(24));

		int nLogs = SableUI::Console::m_Logs.size();

		for (int i = std::max(nLogs - 18, 0); i < nLogs; i++)
		{
			const auto& logData = SableUI::Console::m_Logs[i];
			SableUI::Colour logColour = rgb(255, 255, 255);

			switch (logData.type)
			{
			case SableUI::LogType::SBUI_LOG: logColour = rgb(0, 255, 0); break;
			case SableUI::LogType::SBUI_WARNING: logColour = rgb(255, 255, 0); break;
			case SableUI::LogType::SBUI_ERROR: logColour = rgb(255, 0, 0); break;
			default: logColour = rgb(255, 255, 255); break;
			}

			Text(logData.message, textColour(logColour));
		}
	}

	void OnUpdate(const SableUI::UIEventContext& ctx) override
	{
		if (nLogs != SableUI::Console::m_Logs.size())
		{
			setNlogs(SableUI::Console::m_Logs.size());
		}
	}

private:
	useState(nLogs, setNlogs, int, 0);
};

int main(int argc, char** argv)
{
	SableUI::PreInit(argc, argv);
	//SableUI::SetBackend(SableUI::Backend::Vulkan);
	SableUI::Window* mainWindow = SableUI::Initialise("SableUI Test", 1600, 900);
	SableUI::SetMaxFPS(60);

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
						PanelWith(ImageView, "3.webp", 128, 128);
					}
				}
			}
			PanelWith(ConsoleView);

		}
		Panel();
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

/* TO ADD
| | vulkan
| | debug window
|x| better event system
|x| make text rerenders more efficient

|x| rect
	| | rounding
	| | border

|x| text
	|x| unicode
	|x| font size
	|x| word wrap
	|x| inline style
	|x| colour
	|x| alignment
	|x| line Height
	|x| truncation
	|x| black text

|x| image
	|x| jpg, jpeg, png, ...
	|x| webp

|x| div
| | scroll view
| | z-stack

| | button
| | text input
	| | ctrl+z
	| | unicode input

| | checkbox
| | slider
	| | agnostic

| | dropdown
| | radio
	| | by selection
	| | by checkmark

| | switch

| | tooltip
| | progress bar
| | seperator
| | modal
| | context menu
| | tab view

| | render target panel
*/
////////////////////////////////////