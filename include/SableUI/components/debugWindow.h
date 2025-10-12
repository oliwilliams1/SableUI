#pragma once
#include "SableUI/SableUI.h"
#include "SableUI/components/treeView.h"
#include "SableUI/element.h"
#include "SableUI/memory.h"

using namespace SableMemory;
class ImageView2 : public SableUI::BaseComponent
{
public:
	explicit ImageView2(std::string  path, const int width = 128, const int height = 128)
		: SableUI::BaseComponent(), m_path(std::move(path)), width(width), height(height) {};

	void Layout() override
	{
		Image(m_path, w(width) h(height) centerXY
			onHover([&]() { setText(U"unicode test"); })
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

namespace SableUI
{
	inline SableString GetPanelName(BasePanel* panel)
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
			GenerateTreeView(window->GetRoot());
		}

		TreeNode BuildTreeFromPanel(BasePanel* panel)
		{
			if (panel == nullptr) return TreeNode("Null");

			TreeNode node(GetPanelName(panel));

			for (BasePanel* child : panel->children)
			{
				node.children.push_back(BuildTreeFromPanel(child));
			}

			if (Panel* contentPanel = dynamic_cast<Panel*>(panel))
			{
				node.children.push_back(BuildTreeFromElement(contentPanel->GetComponent()->GetRootElement()));
			}

			return node;
		}

		TreeNode BuildTreeFromElement(Element* element)
		{
			if (element == nullptr) return TreeNode("Null");

			TreeNode node(ElementTypeToString(element->type));

			for (Child* child : element->children)
			{
				Element* el = (Element*)*child;
				node.children.push_back(BuildTreeFromElement(el));
			}

			return node;
		}

		void GenerateTreeView(BasePanel* root)
		{
			//tree = BuildTreeFromPanel(root);
		}

		void Layout() override
		{
			SableUI_Log("Re rendering tree view");
			Text(std::to_string(n));
			Component(ImageView2, , "3.png");
		}

	private:
		useState(n, setN, int, 0);
		Window* m_window = nullptr;
		bool windowUpdated = false;
	};
}