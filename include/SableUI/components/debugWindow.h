#pragma once
#include "SableUI/SableUI.h"
#include "SableUI/element.h"

namespace SableUI
{
	SableString GetPanelName(BasePanel* panel)
	{
		switch (panel->type)
		{
		case SableUI::PanelType::BASE:
			return "content panel";
			break;
		case SableUI::PanelType::HORIZONTAL:
			return "h-splitter";
			break;
		case SableUI::PanelType::VERTICAL:
			return "v-splitter";
			break;
		case SableUI::PanelType::UNDEF:
			return "undefined panel type";
			break;
		case SableUI::PanelType::ROOTNODE:
			return "root";
			break;
		default:
			return "unknown panel type";
			break;
		}
	}

	SableString ElementTypeToString(ElementType type)
	{
		switch (type)
		{
		case SableUI::ElementType::UNDEF:
			return "undefined element";
			break;
		case SableUI::ElementType::RECT:
			return "rect";
			break;
		case SableUI::ElementType::IMAGE:
			return "image";
			break;
		case SableUI::ElementType::TEXT:
			return "text";
			break;
		case SableUI::ElementType::DIV:
			return "div";
			break;
		case SableUI::ElementType::TEXT_U32:
			return "text u32";
			break;
		default:
			return "unknown element type";
			break;
		}
	}

	class DebugWindowView : public SableUI::BaseComponent
	{
	public:
		DebugWindowView(SableUI::Window* window) : SableUI::BaseComponent()
		{
			m_window = window;
		}

		void Layout() override
		{
			if (pollingHeartbeat)
			{
				Text("Polling heartbeat |x|\n" + std::to_string(n), onClick([&]() { setPollingHeartbeat(!pollingHeartbeat); }));
			}
			else
			{
				Text("Polling heartbeat | |", onClick([&]() { setPollingHeartbeat(!pollingHeartbeat); }));
			}
		}

		void OnUpdate(const UIEventContext& ctx) override
		{
			if (pollingHeartbeat)
			{
				setN(n + 1);
			}
		}

	private:
		useState(pollingHeartbeat, setPollingHeartbeat, bool, false);
		useState(n, setN, int, 0);
		Window* m_window = nullptr;
	};
}