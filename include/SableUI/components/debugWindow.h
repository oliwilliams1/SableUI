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
			SableString indent = std::string(2 * depth, ' ');
			Text(indent + ElementTypeToString(element->type));

			for (Child* child : element->children)
			{
				Element* el = (Element*)*child;
				DrawElementTree(el, depth + 1);
			}
		}

		void DrawPanelTree(BasePanel* panel, int depth = 0)
		{
			SableString indent = std::string(2 * depth, ' ');
			Text(indent + GetPanelName(panel));
			depth++;

			for (BasePanel* child : panel->children)
			{
				DrawPanelTree(child, depth);
			}

			if (!panel->children.empty()) return;

			if (ContentPanel* p = dynamic_cast<ContentPanel*>(panel))
			{
				DrawElementTree(p->GetComponent()->GetRootElement(), depth);
			}
		}

		void Layout() override
		{
			rootElement->setPadding(4);
			if (pollingHeartbeat)
			{
				Text("Polling heartbest |x|", onClick([&]() { setPollingHeartbeat(false); }));
				Text(std::to_string(n));
			}
			else
			{
				Text("Polling heartbest | |", onClick([&]() { setPollingHeartbeat(true); }));
			}
			Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

			Text("Open/close mem diagnostings", onClick([&]() { setMemoryDebugger(!memoryDebugger); }));

			if (memoryDebugger)
			{
				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));
				Text("Base Panels: " + std::to_string(BasePanel::GetNumInstances()));
				Text("Root Panels: " + std::to_string(RootPanel::GetNumInstances()));
				Text("Splitter Panels: " + std::to_string(SplitterPanel::GetNumInstances()));
				Text("Content Panels: " + std::to_string(ContentPanel::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

				Text("Components: " + std::to_string(BaseComponent::GetNumInstances()));
				Text("Elements: " + std::to_string(Element::GetNumInstances()));
				Text("Virtual Elements: " + std::to_string(VirtualNode::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

				Text("Drawable Base: " + std::to_string(DrawableBase::GetNumInstances()));
				Text("Drawable Text: " + std::to_string(DrawableText::GetNumInstances()));
				Text("Drawable Rect: " + std::to_string(DrawableRect::GetNumInstances()));
				Text("Drawable Splitter: " + std::to_string(DrawableSplitter::GetNumInstances()));
				Text("Drawable Image: " + std::to_string(DrawableImage::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

				Text("Text: " + std::to_string(_Text::GetNumInstances()));
				Text("Textures: " + std::to_string(Texture::GetNumInstances()));
				Text("Strings: " + std::to_string(String::GetNumInstances()));

				Rect(mx(2) mt(8) mb(4) h(1) w_fill bg(67, 67, 67));

				Text("Font Packs: " + std::to_string(FontPack::GetNumInstances()));
				Text("Font Ranges: " + std::to_string(FontRange::GetNumInstances()));
			}

			//DrawPanelTree(m_window->GetRoot());
		}

		void OnUpdate(const UIEventContext& ctx) override
		{
			if (pollingHeartbeat)
			{
				setN(n + 1);
			}
		}

	private:
		useState(pollingHeartbeat, setPollingHeartbeat, bool, true);
		useState(memoryDebugger, setMemoryDebugger, bool, false);
		useState(n, setN, int, 0);
		Window* m_window = nullptr;
	};
}