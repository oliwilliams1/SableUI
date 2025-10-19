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

		void DrawElementTree(Element* element, int depth)
		{
			SableString indent = std::string(2 * depth, ' ').c_str();
			Text(indent + ElementTypeToString(element->type));

			for (Child* child : element->children)
			{
				Element* el = (Element*)*child;
				DrawElementTree(el, depth + 1);
			}
		}

		void DrawPanelTree(BasePanel* panel, int depth = 0)
		{
			SableString indent = std::string(2 * depth, ' ').c_str();
			Text(indent + GetPanelName(panel));
			depth++;

			for (BasePanel* child : panel->children)
			{
				DrawPanelTree(child, depth);
			}

			if (!panel->children.empty()) return;

			if (Panel* p = dynamic_cast<Panel*>(panel))
			{
				DrawElementTree(p->GetComponent()->GetRootElement(), depth);
			}
		}

		void Layout() override
		{
			DrawPanelTree(m_window->GetRoot());
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